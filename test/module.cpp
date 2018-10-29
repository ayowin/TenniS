//
// Created by kier on 2018/10/16.
//

#include <module/graph.h>
#include <module/module.h>
#include <global/setup.h>
#include <runtime/workbench.h>
#include <global/operator_factory.h>
#include <utils/ctxmgr.h>

#include <cstring>

class Sum : public ts::Operator {
public:
    virtual int run(ts::Stack &stack) {
        int input_num = stack.size();
        std::vector<ts::Tensor::Prototype> output;
        this->infer(stack, output);
        assert(output[0].type() == ts::FLOAT32);
        stack.push(output[0]);
        auto &sum = *stack.index(-1);
        std::memset(sum.data<float>(), 0, sum.count() * sizeof(float));
        for (int i = 0; i < input_num; ++i) {
            for (int j = 0; j < sum.count(); ++j) {
                sum.data<float>()[j] += stack.index(i)->data<float>()[j];
            }
        }
        return 1;
    }

    virtual int infer(ts::Stack &stack, std::vector<ts::Tensor::Prototype> &output) {
        if (stack.size() == 0) throw ts::Exception("Can not sum on empty inputs");
        for (int i = 1; i < stack.size(); ++i) {
            if (stack.index(i)->sizes() != stack.index(0)->sizes()) {
                ts::Exception("Can not sum mismatch size inputs");
            }
        }
        output.resize(1);
        output[0] = stack.index(0)->proto();
        return 1;
    }
};

TS_REGISTER_OPERATOR(Sum, ts::CPU, "sum")

namespace ts {

    namespace op {
        Node param(const std::string &name) {
            auto &g = ctx::ref<Graph>();
            return g.make<OP>(OP::Parameter, name);
        }
        Node sum(const std::string &name, const std::vector<Node> &nodes) {
            auto &g = ctx::ref<Graph>();
            Node result = g.make<OP>("sum", name);
            Node::Link(result, nodes);
            return result;
        }
    }

    Node block(const std::string &prefix, Node in) {
        auto conv = op::sum(prefix + "/" + "a", {in});
        auto bn = op::sum(prefix + "/" + "b", {conv});
        auto shortcut = op::sum(prefix + "/" + "out", {in, bn});
        return shortcut;
    }

    Node block_n_times(const std::string &prefix, Node in, int n) {
        auto blob = in;
        for (int i = 0; i < n; ++i) {
            blob = block(prefix + "/" + "block_" + std::to_string(i), blob);
        }
        return blob;
    }
}

int main()
{
    using namespace ts;
    setup();

    // build graph
    Graph g;
    ctx::bind<Graph> _graph(g);

//    auto a = op::in("input");
//    auto block = block_n_times("block", a, 1000);
//    auto output = op::sum("output", {block});
//    auto a = op::in("a");
//    auto b = op::sum("b", {a});
//    auto c = op::sum("c", {b});
//    auto d = op::sum("d", {a, c});

    auto a = op::param("a");
    auto b = op::param("b");

    auto c = op::sum("c", {a, b});

    // setup module
    std::shared_ptr<Module> m = std::make_shared<Module>();
    m->load(g);

    std::cout << "Input nodes:" << std::endl;
    for (auto &node : m->inputs()) {
        std::cout << node.ref<OP>().op << ":" << node.ref<OP>().name << std::endl;
    }

    std::cout << "Output nodes:" << std::endl;
    for (auto &node : m->outputs()) {
        std::cout << node.ref<OP>().op << ":" << node.ref<OP>().name << std::endl;
    }

    // run workbench
    ComputingDevice device(CPU, 0);
    // Workbench bench(device);

    auto bench = Workbench::Load(m, device);

    Tensor input_a(FLOAT32, {1});
    Tensor input_b(FLOAT32, {1});

    input_a.data<float>()[0] = 1;
    input_b.data<float>()[0] = 3;

    bench->input("a", input_a);
    bench->input("b", input_b);

    bench->run();

    auto output_c = bench->output("c");

    std::cout << "output: " << output_c.data<float>()[0] << std::endl;
}
