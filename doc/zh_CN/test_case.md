# 此文档说明测试用例格式

## 基本说明

`.t` 文件为 `Tensor` 对象序列化后的格式名称，关于 `Tensor` 的二进制格式见：`moduel.md`。

## 文件结构

测试用例完整包含在一个文件夹内：
包含以下几类文件：
1. 算符说明文件。
2. 参数说明文件。
3. 输入说明文件。
4. 输出说明文件。

## 算符说明文件

算符说明文件命名为`0.<op>.txt`，其中`<op>`为算符名称。
文件内容以`text`模型含有三行，
分别为：  
参数说明文件个数；  
输入说明文件个数；  
输出说明文件个数。

例如：
```
2
3
1
```
表示，该算符测试含有2个参数，3个输入，一个输出。

## 参数说明文件

参数说明文件是`.t`格式。命名规则为`1.<param_name>.t`，其中`<param_name>`为参数名。  
文件内容为对应的`Tensor`值。

## 输入说明文件

参数说明文件是`.t`格式。命名规则为`2.input_<index>.t`，其中`<index>`为输入的索引下表，从`0`开始。  
文件内容为对应的`Tensor`值。

## 输出说明文件

参数说明文件是`.t`格式。命名规则为`3.output_<index>.t`，其中`<index>`为输出的索引下表，从`0`开始。  
文件内容为对应的`Tensor`值。

# 测试用例使用说明

1. 首先打开以`0.`开头，并以`.txt`结尾的文件，获取算符名称，和对应的参数、输入、输出个数，用于校验。
2. 构造对应算符，设置保留变量的值，包括`#op`、`#name`、`#output_count`。
3. 读取所有`1\.[^\.]*\.t`文件，并设置算符参数。
4. 打开所有`2\.input_[0-9]*\.t`文件，并按照下标构造输入。
5. 运行算符的`infer`和`run`接口。
6. 打开所有`3\.output_[0-9]*\.t`文件，对比算符给出的输出，对比给出结论。

# 补充说明

测试输出包含几个方面，1. 输出个数，和对应大小、类型是否匹配。2. 输出的内容误差大小。
内容的误差均采用`float`表示，给出最大元素误差，和平均元素误差。
平均元素误差一般不超过`1e-5`，最大元素误差一般不超过`1e-4`。
如果不满足上述测试，则认为该测试不通过。

# 模型文件测试用例
模型文件也有测试用例，包含：  
1. `a.in([0-9]*).out([0-9]*).tsm` 模型文件，其中`$1`表示输入个数，`$2`表示输出个数。
2. `b.input_([0-9]*).t` 表示第`$1`个输入。
3. `c.input_([0-9]*).t` 表示第`$1`个输出。