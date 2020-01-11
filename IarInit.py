import os,sys,re
from xml.dom import minidom

print(os.getcwd())
projectDir = os.path.dirname(os.getcwd()) + os.sep
sys.path.append(os.getcwd())

headerPrefix = '' \
          '{0}' \
          '/* Includes ----------------------------------------------------------*/\n' \
          '#include "Module/Module.h"\n{1}' \
          '/* define ------------------------------------------------------------*/\n' \
          '/* typedef -----------------------------------------------------------*/\n' \
          '/* macro -------------------------------------------------------------*/\n' \
          '/* variables ---------------------------------------------------------*/\n' \
          '/* function prototypes -----------------------------------------------*/\n' \
          '{2}\n'\
          '#endif'

headerDefine = "#ifndef _{0}_H_\n#define _{0}_H_\n\n"

filePrefix = '/* Includes ----------------------------------------------------------*/\n'\
             '{0}' \
             '/* define ------------------------------------------------------------*/\n' \
             '/* typedef -----------------------------------------------------------*/\n' \
             '/* macro -------------------------------------------------------------*/\n' \
             '/* variables ---------------------------------------------------------*/\n' \
             '/* function prototypes -----------------------------------------------*/\n\n' \
             '/*********************************************************************************************\n\n' \
             '    * @brief {1}_Init\n' \
             '    * @param\n' \
             '    * @retval\n' \
             '    * @remark\n\n' \
             '********************************************************************************************/\n'\
             'void {1}_Init()\n'\
             '{2}\n'\
             '/*********************************************************************************************\n\n' \
             '    * @brief {1}_Handle\n' \
             '    * @param\n' \
             '    * @retval\n' \
             '    * @remark\n\n' \
             '********************************************************************************************/\n'\
             'void {1}_Handle()\n'\
             '{2}'

fileFrame = {
    'HardWare': ['Hal'],
    'Handle': ['Conf', 'Service', 'Communication', 'Global']
}


# 1. 新建文件夹以及文件
# 2. 并将文件内前缀添加
includePrefix = "".join("#include \"" + i + "/"+ i + ".h\"\n" for i in fileFrame.keys())

for key,item in fileFrame.items():
    # 创建文件夹
    try:
        print("创建 " + key + " 文件夹")
        os.mkdir(projectDir + key)
    except:
        print("[Warning]" + key + "文件夹已创建，跳过")

    # 建立统一的header文件，并将所有文件的.h文件添加进去
    with open(projectDir + key + os.sep + key + '.h', 'w', encoding='utf8') as f:
        f.write(headerPrefix.format(headerDefine.format(key.upper()),
                                    "".join("#include \"" + key + "/" + i + ".h\"\n" for i in item),
                                    ''))

    for file in item:
        print("创建 " + file + " 文件")
        with open(projectDir + key + os.sep + file + '.c', 'w', encoding='utf8') as f:
            f.write(filePrefix.format(includePrefix, file, "{}"))
        with open(projectDir + key + os.sep + file + '.h', 'w', encoding='utf8') as f:
            f.write(headerPrefix.format(headerDefine.format(file.upper()),"", "void {0}_Init();\nvoid {0}_Handle();\n".format(file)))

# 3. 找到工程ewp文件，并开始dom解析


for c,d,f in os.walk(projectDir):
    for file in f:
        if re.match("[a-zA-Z]*\.ewp$", file):
            projectFile = c + os.sep + file
print(projectFile)
with open(projectFile, 'r', encoding='utf8') as f:
    dom = minidom.parse(f)

    root = dom.documentElement

    print(root.nodeName)
    group = root.getElementsByTagName('group')
    print(group)

    # 4. 将ewp文件dom重新加入新的文件树，并写入

    dirs = [i for i in fileFrame.keys()]
    dirs.append('Module')
    doms = []

    def groupElement(dirName):
        group = dom.createElement('group')
        name = dom.createElement('name')
        name.appendChild(dom.createTextNode(dirName))
        group.appendChild(name)
        return group

    def fileElement(dirName, fileName):
        file = dom.createElement('file')
        name = dom.createElement('name')
        name.appendChild(dom.createTextNode("$PROJ_DIR$/../%s/%s" % (dirName, fileName)))
        file.appendChild(name)
        return file

    for dirName in dirs:
        # 创建dom并设置属性
        group = groupElement(dirName)

        c1,d1,f1 = os.walk(projectDir + dirName).__next__()
        if '.git' in d1:
            d1.remove(".git")
        for dir1Name in d1:
            group1 = groupElement(dir1Name)
            _,_,f2 = os.walk(projectDir+dirName + os.sep + dir1Name).__next__()
            print(f2)
            for file2Name in f2:
                if re.match('.*\.[ch]$', file2Name):
                    group1.appendChild(fileElement('%s/%s' % (dirName, dir1Name), file2Name))
                group.appendChild(group1)
        for file1Name in f1:
            if re.match('.*\.[ch]$', file1Name):
                group.appendChild(fileElement(dirName, file1Name))


        root.appendChild(group)

with open(projectFile, 'w+', encoding='utf8') as f:
    dom.writexml(f, indent='', addindent='\t', newl='\n', encoding='UTF-8')

#
# print(xmlContent)