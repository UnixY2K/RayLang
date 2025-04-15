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
            "Name": className,
            "Fields": fields
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
        headerFile.write(f"class {baseName} {{\n")
        headerFile.write("  public:\n")
        headerFile.write(f"\tvirtual ~{baseName}() = default;\n")
        headerFile.write("};\n\n")

        for clazz in processedClasses:
            result = defineType(baseName, clazz)
            headerFile.write(result)
            headerFile.write("\n")

        headerFile.write("\n")

        headerFile.write("\n")
        headerFile.write(defineVisitor(baseName, processedClasses))
        headerFile.write("\n")

        headerFile.write("} // namespace ray::compiler::ast\n")


def defineType(baseName: str, clazz: dict[str]):
    stringList = list[str]()

    stringList.append(
        f"class {clazz["Name"]} : public {baseName} {{\n")
    stringList.append(f"  public:\n")
    for field in clazz["Fields"]:
        stringList.append(
            f"\t{field["Type"]} {field["Name"]};\n")
    stringList.append("\n")

    # define constructor
    stringList.append(f"\t{clazz["Name"]}(")
    constructorParams = list[str]()
    for field in clazz["Fields"]:
        constructorParams.append(
            f"{field["Type"]} {field["Name"]}")

    stringList.append(f",\n\t{" " * 8}".join(constructorParams))
    stringList.append(f")\n\t{" " * 4}: ")
    initializerList = list[str]()
    for field in clazz["Fields"]:
        initializerList.append(f"{field["Name"]}(std::move({field["Name"]}))")

    stringList.append(", ".join(initializerList))
    stringList.append(" {}\n\n")

    stringList.append("};")

    return "".join(stringList)

def defineVisitor(baseName: str, types: dict):
    stringList = list[str]()
    stringList.append(f"class {baseName}Visitor {{\n")
    stringList.append("  public:\n")
    for clazz in types:
        className = clazz["Name"]
        stringList.append(f"\tvirtual void visit{className}{baseName}({className}& value) = 0;\n")
    stringList.append(f"\tvirtual ~{baseName}Visitor() = default;\n")
    stringList.append("};\n")

    return "".join(stringList)



def main():
    outputDir = "./RayC/include/ray/compiler/ast"
    defineAst(outputDir, "Expression", ["any", "memory", "vector", "ray/compiler/lexer/token.hpp"], [
        "Ternary		= std::unique_ptr<Expression> cond, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right",
        "Assign			= Token name, std::unique_ptr<Expression> value",
        "Binary			= std::unique_ptr<Expression> left, Token op, std::unique_ptr<Expression> right",
        "Call			= std::unique_ptr<Expression> callee, Token paren, std::vector<std::unique_ptr<Expression>> arguments",
        "Get			= std::unique_ptr<Expression> object, Token name",
        "Grouping		= std::unique_ptr<Expression> expression",
        "Literal		= std::any value",
        "Logical		= std::unique_ptr<Expression> left, Token op, std::unique_ptr<Expression> right",
        "Set			= std::unique_ptr<Expression> object, Token name, std::unique_ptr<Expression> value",
        "Unary			= Token op, std::unique_ptr<Expression> right",
        "Variable		= Token name",
        "Type           = Token type",
        "Parameter	    = Token name, Type type",
    ])
    defineAst(outputDir, "Statement",
            ["memory",
             "vector",
             "ray/compiler/lexer/token.hpp",
             "ray/compiler/ast/expression.hpp"
            ],
            [
                "Block			= std::vector<std::unique_ptr<Statement>> statements",
                "TerminalExpr	= std::unique_ptr<Expression> expression",
                "ExpressionStmt	= std::unique_ptr<Expression> expression",
                "Function		= Token name, std::vector<Parameter> params, std::vector<std::unique_ptr<Statement>> body, Type returnType",
                "If				= std::unique_ptr<Expression> condition, std::unique_ptr<Statement> thenBranch, std::unique_ptr<Statement> elseBranch",
                "Jump			= Token keyword, std::unique_ptr<Expression> value",
                "Var			= Token name, std::unique_ptr<Expression> initializer",
                "While			= std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body"
            ])


if __name__ == "__main__":
    sys.exit(main())
