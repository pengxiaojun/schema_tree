# schema_tree
根据schema描述文件生成一棵schema树,该schema树可用来生成UI, 可用作模块间数据交换格式

### schema描述文件说明
schema描述文件一种类似xml的自描述语言,比xml更简洁，采用json格式, 格式如下:

`
{
	'name':'root', #[required]对像名称.并且name为root的对像表示根对象, 每一个schema描述文件都必须包含一个根对像
	'attr':        #[required]对像属性 
	[
		'name':'attribute name', #[required]属性名称
		'type':'int',            #[required]属性类型. 内置枚举值: int, string, bool, guid, object, 也可支持其它对像. 见例子2
		'key':false,             #[optional]. bool类型, 属性标识符. type为object时必填
		'txt':false,             #[optional]. bool类型, 属性文本. type为object时必填
		'plural':false,          #[optional]. bool类型, 属性是否为复数形式. (可以界面上生成comobox, listbox)
		'hide': false,           #[optional]. 默认为false, 是否隐藏该属性. (设置为true, 该元素会在界面上不可见)
		'data':                  #[optional]属性静态数据
		{
			'min' : 1,           #最小取值
			'max' : 100,         #最大取值
			'def' : 90,          #数值或字符类型，默认取值
			'set':               #静态数据集
			[
				{
					'value' : 1, #数值或字符类型, 静态数据项值.
					'event'      #数据项值关联的事件.主要用于界面在选择某一项数据时，禁用或者隐藏其它界面元素
					{
						'act':'hide', #事件动作. 枚举: hide, disable.分别表示隐藏或者禁用	
						'type':'path',#事件类型. 枚举: path[路径事件，会扩散到指定的target上], cascade[级联事件,会扩散到该对像下的所有子对像]
						'target':[11, 12...] #type为path时需指定的path
					}
				}
			]
		}
	]
}

`

### schema例子1
单对像工作:
`
{
	'name':'root',
	'attr':
	[
		{
			'name':'Scores', 
			'type':'int',
			'data':
			{
				'min':1,
				'max':100,
				'def':90,
				'set':
				[
					{'value':1,'event':{'act':'hide','type':'path',target':['11']}},
					{'value':2,'event':{'act':'hide','type':'cascade'}}
				]
			}
		},
		{
			'name':'Disk State',
			'type':'string',
			'hide':true,
			'format':'Disk Quota %d'
		}
	]
}
`

### schema例子2
多对像协同工作
`
{
	'name':'birthday',
	'attr':
	[
		{'name':'Year', 'type':'int'},
		{'name':'Month', 'type':'int'},
		{'name':'Day', 'type':'int'}
	]
}

{
	'name':'root',
	'attr':
	[
		{
			'name':'User name',
			'type':'string'
		},
		{
			'name':'Birthday',
			'type':'birthday'  #birtyday 类型, 会引用上面name为birthday的对像
		}
		...
	]
}

### 关于路径描述
schema中每一个对像的属性都会有一个path值， 表示该属性在整个schema树上的路径. root对像从1开始.
如示例2, 各个属性的path值对应如下: 

root = 1
root.username=11
root.birthday=12
root.birthday.year = 121
root.birthday.month =122
root.birthday.day =123


`

### schema支持的功能
- 界面自动生成
- 静态数据存储
- 动态数据处理

