#!/usr/bin/env python3
import sys
import os


def preprocessTypes(baseName: str, types: list[str]):
    processedClasses = list[dict]()
    for Type in types:
        (className, fieldLines) = [x.strip() for x in Type.split("=")]
        fields = list[dict]()
        for field in fieldLines.split(","):
            value = field.strip().split(" ")
            if len(value) == 2:
                (fieldType, name) = value
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


def defineAst(outputDir: str, baseName: str, requiredHeaders: list[str], definitions: list[str], types: list[str]):
    filePath = os.path.join(outputDir, f"{baseName.lower()}.hpp")
    processedClasses = preprocessTypes(baseName, types)
    with open(filePath, "w") as headerFile:
        headerFile.write("#pragma once\n")
        for header in requiredHeaders:
            headerFile.write(f"#include <{header}>\n")
        headerFile.write("\n")
        headerFile.write("namespace ray::compiler::ast {\n")
        headerFile.write("\n")
        for definition in definitions:
            (name, value) = [x.strip() for x in definition.split("=")]
            headerFile.write(f"using {name} = {value};\n")
        if len(definitions) != 0:
            headerFile.write("\n")

        for clazz in processedClasses:
            headerFile.write(f"class {clazz["Name"]};")
            headerFile.write("\n")

        headerFile.write("\n")
        headerFile.write(defineVisitor(baseName, processedClasses))
        headerFile.write("\n")

        headerFile.write(f"class {baseName} {{\n")
        headerFile.write("  public:\n")
        headerFile.write(f"\tvirtual void visit({baseName}Visitor& visitor) const = 0;\n")
        headerFile.write(f"\tvirtual const std::string_view variantName() const = 0;\n")
        headerFile.write(f"\tvirtual const Token& getToken() const = 0;\n")
        headerFile.write(f"\tvirtual ~{baseName}() = default;\n")
        headerFile.write("};\n\n")

        for clazz in processedClasses:
            result = defineType(baseName, clazz)
            headerFile.write(result)
            headerFile.write("\n")

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
    # append to the fields a Token Location
    stringList.append(
            f"\tToken token;\n")
    stringList.append("\n")

    # define constructor
    stringList.append(f"\t{clazz["Name"]}(")
    constructorParams = list[str]()
    for field in clazz["Fields"]:
        constructorParams.append(
            f"{field["Type"]} {field["Name"]}")
    constructorParams.append("Token token")

    stringList.append(f",\n\t{" " * 8}".join(constructorParams))
    if (len(clazz["Fields"]) > 0):
        stringList.append(f"):\n\t\t")
        initializerList = list[str]()
        for field in clazz["Fields"]:
            initializerList.append(f"{field["Name"]}(std::move({field["Name"]}))")
        initializerList.append("token(std::move(token))")
        stringList.append(",\n\t\t".join(initializerList))
    else:
        stringList.append(")")
    stringList.append(" {}\n\n")
    stringList.append(f"\tvoid visit({baseName}Visitor& visitor) const override {{\n\t\tvisitor.visit{clazz["Name"]}{baseName}(*this);\n\t}}\n\n")
    stringList.append(f"\tconst std::string_view variantName() const override {{ return \"{clazz["Name"]}\"; }}\n\n")
    stringList.append(f"\tconst Token& getToken() const override {{ return token; }};\n")

    stringList.append("};")

    return "".join(stringList)

def defineVisitor(baseName: str, types: dict):
    stringList = list[str]()
    stringList.append(f"class {baseName}Visitor {{\n")
    stringList.append("  public:\n")
    for clazz in types:
        className = clazz["Name"]
        stringList.append(f"\tvirtual void visit{className}{baseName}(const {className}& value) = 0;\n")
    stringList.append(f"\tvirtual ~{baseName}Visitor() = default;\n")
    stringList.append("};\n")

    return "".join(stringList)



def main():
    outputDir = "./RayC/include/ray/compiler/ast"
    defineAst(outputDir, "Expression",
            ["memory",
             "vector",
             "optional",
             "ray/compiler/lexer/token.hpp",
             "ray/compiler/ast/intrinsic.hpp"
            ],
            [],
            ["Variable		= Token name",
             "Intrinsic		= Token name, IntrinsicType intrinsic",
             "Assign		= std::unique_ptr<Expression> lhs, Token assignmentOp, std::unique_ptr<Expression> rhs",
             "Binary		= std::unique_ptr<Expression> left, Token op, std::unique_ptr<Expression> right",
             "Call			= std::unique_ptr<Expression> callee, Token paren, std::vector<std::unique_ptr<Expression>> arguments",
             "IntrinsicCall	= std::unique_ptr<Intrinsic> callee, Token paren, std::vector<std::unique_ptr<Expression>> arguments",
             "Get			= std::unique_ptr<Expression> object, Token name",
             "Grouping		= std::unique_ptr<Expression> expression",
             "Literal		= Token kind, std::string value",
             "Logical		= std::unique_ptr<Expression> left, Token op, std::unique_ptr<Expression> right",
             "Set			= std::unique_ptr<Expression> object, Token name, Token assignmentOp, std::unique_ptr<Expression> value",
             "Unary			= Token op, bool isPrefix, std::unique_ptr<Expression> expr",
             "ArrayAccess	= std::unique_ptr<Expression> array, std::unique_ptr<Expression> index",
             "Type			= Token name, bool isMutable, bool isPointer, std::optional<std::unique_ptr<Type>> subtype",
             "Cast			= std::unique_ptr<Expression> expression, Type type",
             "Parameter		= Token name, Type type",
            ])
    defineAst(outputDir, "Statement",
            ["memory",
             "unordered_map",
             "vector",
             "optional",
             "ray/compiler/lexer/token.hpp",
             "ray/compiler/ast/expression.hpp"
            ],
            ["CompDirectiveAttr = std::unordered_map<std::string, std::string>"],
            ["Block			= std::vector<std::unique_ptr<Statement>> statements",
             "TerminalExpr	= std::optional<std::unique_ptr<Expression>> expression",
             "ExpressionStmt= std::unique_ptr<Expression> expression",
             "Function		= Token name, bool publicVisibility, std::vector<Parameter> params, std::optional<Block> body, Type returnType",
             "If			= std::unique_ptr<Expression> condition, std::unique_ptr<Statement> thenBranch, std::optional<std::unique_ptr<Statement>> elseBranch",
             "Jump			= Token keyword, std::optional<std::unique_ptr<Expression>> value",
             "Var			= Token name, Type type, bool is_mutable, std::optional<std::unique_ptr<Expression>> initializer",
             "While			= std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body",
             "Struct		= Token name, bool publicVisibility, bool declaration, std::vector<Var> members, std::vector<bool> memberVisibility",
             "CompDirective	= Token name, CompDirectiveAttr values, std::unique_ptr<Statement> child"
            ])


if __name__ == "__main__":
    sys.exit(main())
