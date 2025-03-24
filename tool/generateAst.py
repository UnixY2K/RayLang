#!/usr/bin/env python3
import sys
import os

def preprocessTypes(baseName: str, types: list[str]):
	processedClasses = list[dict]()
	for Type in types:
		(className, fieldLines) = [x.strip() for x in Type.split("=")]
		fields = list[dict]()
		for field in fieldLines.split(","):
			(fieldType, name) = field.strip().split(" ")
			fields.append({
				"Type": fieldType.strip(),
				"Name": name.strip()
			})
		processedClass = {
			"Name"   : className,
			"Fields" : fields
		}
		processedClasses.append(processedClass)
	return processedClasses

def defineAst(outputDir: str, baseName: str, requiredHeaders: list[str], types: list[str]):
	filePath = os.path.join(outputDir, f"{baseName.lower()}.hpp")
	processedClasses = preprocessTypes(baseName, types)
	with open(filePath, "w") as headerFile:
		headerFile.write("#pragma once\n")
		for header in requiredHeaders:
			headerFile.write(f"#include <{header}>\n")
		headerFile.write("\n")
		headerFile.write("namespace ray::compiler::ast {\n")
		headerFile.write("\n")
		headerFile.write(f"template <class T> class {baseName}Visitor;\n")
		headerFile.write("\n")
		headerFile.write(f"template <class T> class {baseName} {{\n")
		headerFile.write("  public:\n")
		headerFile.write(f"\tvirtual T accept({baseName}Visitor<T> &visitor) = 0;\n")
		headerFile.write("};\n\n")

		for clazz in processedClasses:
			result = defineType(baseName, clazz)
			headerFile.write(result)

		headerFile.write("\n")
		headerFile.write(f"template <class T> class {baseName}Visitor {{\n")
		for clazz in processedClasses:
			headerFile.write(f"\tvirtual T visit{clazz["Name"]}Expression({clazz["Name"]}<T> &ternary) = 0;\n")
		headerFile.write("};\n\n")


		headerFile.write("} // namespace ray::compiler::ast\n")

def defineType(baseName: str, clazz: dict[str]):
	stringList = list[str]()

	stringList.append(f"template <class T> class {clazz["Name"]} : {baseName}<T> {{\n")
	stringList.append(f"  public:\n")
	for field in clazz["Fields"]:
		stringList.append(f"\tstd::unique_ptr<{field["Type"]}> {field["Name"]};\n")
	stringList.append("\n")

	# define constructor
	stringList.append(f"\t{clazz["Name"]}(")
	constructorParams = list[str]()
	for field in clazz["Fields"]:
		constructorParams.append(f"std::unique_ptr<{field["Type"]}> {field["Name"]}")

	stringList.append(f",\n\t{" " * 8}".join(constructorParams))
	stringList.append(f")\n\t{" " * 4}: ")
	initializerList = list[str]()
	for field in clazz["Fields"]:
		initializerList.append(f"{field["Name"]}({field["Name"]})")

	stringList.append(", ".join(initializerList))
	stringList.append(" {}\n\n")

	# define accept method
	stringList.append(f"\tT accept({baseName}Visitor<T> &visitor) override {{\n")
	stringList.append(f"\t\treturn visitor.visit{clazz["Name"]}{baseName}(this);\n")
	stringList.append("\t}\n")

	stringList.append("};\n")
	return "".join(stringList)

def defineVisitor(baseName: str, types: dict):
	stringList = list[str]()
	stringList.append("class ${baseName}Visitor {\n")
	for typeValue in types:
		className = typeValue.Name
		stringList.append(f"\tvisit${className}${baseName}([${className}]`$${className}) {{}}")

	stringList.append("}`n")

	return "".join(stringList)


def main():
	outputDir = "./RayC/include/ray/compiler/ast"
	defineAst(outputDir, "Expression", ["memory"], [
	"Ternary		= Expression<T> cond, Expression<T> left, Expression<T> right",
	])


if __name__ == "__main__":
	sys.exit(main())
