#include <kernels/cpu/concat.h>
#include <core/tensor_builder.h>

namespace ts {

	void Concat::init()
	{
		supper::init();

		auto dim_tensor = get("dim");
		m_dim = tensor::to_int(dim_tensor);

		if (m_dim < 0)
			throw ts::Exception("Concat dim should be greater than 0!");

	}

	int Concat::run(ts::Stack &stack)
	{
		int input_num = stack.size();
		std::vector<ts::Tensor::Prototype> output;

		this->infer(stack, output);
		stack.push(output[0], memory_device());

		int type_len = ts::type_bytes(stack.index(0)->dtype());

		bool flag;
		if (type_len == 1)
		{
			flag = concat<unsigned char>(stack, input_num);
		}
		else if (type_len == 2)
		{
			flag = concat<unsigned short>(stack, input_num);
		}
		else if (type_len == 4)
		{
			flag = concat<float>(stack, input_num);
		}
		else if (type_len == 8)
		{
			flag = concat<double>(stack, input_num);
		}
		return flag ? 1:0;
	}

	int Concat::infer(ts::Stack &stack, std::vector<ts::Tensor::Prototype> &output)
	{
		int input_num = stack.size();
		if (input_num == 0)
			throw ts::Exception("Can not concat on empty inputs!");
		if (input_num == 1)
		{
			output.resize(1);
			output[0] = ts::Tensor::Prototype(stack.index(0)->dtype(), stack.index(0)->sizes());
		}
		
		Shape output_shape(stack.index(0)->sizes());

		if (m_dim > output_shape.size())
			throw ts::Exception("Concat dim should be less than input dim!");

		int num_dims = output_shape.size();
		int concat_dim_output_num = output_shape[m_dim];

		for (int i = 1; i < input_num; i++)
		{
			auto shape = stack.index(i)->sizes();
			if (shape.size() != num_dims)
				throw ts::Exception("All inputs must have the same dims!");
			for (int j = 0; j < shape.size(); j++)
			{
				if (j == m_dim)
					continue;
				if (shape[j] != output_shape[j])
					throw ts::Exception("All inputs must have the same shape, except at concat_axis!");
			}
			concat_dim_output_num += shape[m_dim];
		}
		
		output_shape[m_dim] = concat_dim_output_num;

		output.resize(1);
		output[0] = ts::Tensor::Prototype(stack.index(0)->dtype(), output_shape);
	}

	template<typename T>
	bool Concat::concat(ts::Stack &stack,int input_num)
	{
		auto input_shape = stack.index(0)->sizes();
		ts::Tensor& output_tensor = *stack.index(-1);
		auto output_shape = output_tensor.sizes();
		T* output_data = output_tensor.sync(memory_device()).data<T>();
		if (output_data == nullptr)
			return false;

		int num_concats = 1;              //concats num of each featureMap
		int bottom_concat_size = 1;       //size of each concat in bottom
		int output_concat_axis = output_shape[m_dim];
		
		if (m_dim == 0)
			num_concats = 1;
		else
		{
			for (int i = 0; i < m_dim; i++)
			{
				num_concats *= input_shape[i];
			}
		}

		for (int i = m_dim + 1; i < input_shape.size(); i++)
		{
			bottom_concat_size *= input_shape[i];
		}

		int offset_concat_axis = 0;
		//for (int i = 0; i < input_num; i++)
		//{
		//	const T* input_data = stack.index(i)->sync(memory_device()).data<T>();
		//	int input_concat_axis = stack.index(i)->sizes()[m_dim];
		//	for (int j = 0; j < num_concats; j++)
		//	{
		//		::memcpy(output_data + (j * output_concat_axis + offset_concat_axis)* bottom_concat_size,
		//			input_data + j * input_concat_axis * bottom_concat_size, 
		//			input_concat_axis * bottom_concat_size * sizeof(T));
		//	}
		//	offset_concat_axis += input_concat_axis;
		//}
		int output_index = 0;
		for (int i = 0; i < input_num; i++)
		{
			const T* input_data = stack.index(i)->sync(memory_device()).data<T>();
			int input_concat_axis = stack.index(i)->sizes()[m_dim];
			int input_index = 0;
			for (int j = 0; j < num_concats; j++)
			{
				int output_index_temp = output_index;
				int input_index_temp = input_index;
				for (int num = 0; num < input_concat_axis * bottom_concat_size; num++)
				{
					output_data[output_index_temp++] = input_data[input_index_temp++];
				}
				input_index += input_concat_axis * bottom_concat_size;
				output_index += output_concat_axis * bottom_concat_size;
			}
			offset_concat_axis += input_concat_axis;
			output_index = offset_concat_axis * bottom_concat_size;
		}

		return true;
	}
}