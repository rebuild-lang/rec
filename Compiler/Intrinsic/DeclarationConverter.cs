using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using REC.AST;
using REC.Parser;
using REC.Scope;
using REC.Tools;

namespace REC.Intrinsic
{
    using NetTypes = Dictionary<Type, IModuleDeclaration>;

    class DeclarationConverter
    {
        readonly NetTypes _netTypes = new NetTypes();
        const string SizeTypeName = "u64";

        public static void BuildScope(IScope scope, IIntrinsicDict intrinsicDict) {
            var converter = new DeclarationConverter();
            var declared = converter.DeclareIntrinsics(intrinsicDict, scope);
            converter.ConvertNetTypes(declared, intrinsicDict);
        }

        void ConvertNetTypes(IEnumerable<IExpression> expressions, IIntrinsicDict intrinsicDict) {
            foreach (var expression in expressions) {
                if (expression is FunctionDeclaration functionDeclaration
                    && intrinsicDict[functionDeclaration.Name] is IFunctionIntrinsic intrinsic) {
                    functionDeclaration.LeftArguments = TypeToArguments(intrinsic.LeftArgumentsType);
                    functionDeclaration.RightArguments = TypeToArguments(intrinsic.RightArgumentsType);
                    functionDeclaration.Results = TypeToArguments(intrinsic.ResultType);
                }
            }
        }

        IList<IExpression> DeclareIntrinsics(IEnumerable<IIntrinsic> intrinsics, IScope scope) {
            return intrinsics.Select(intrinsic => (IExpression) DeclareIntrinsic((dynamic) intrinsic, scope)).ToList();
        }

        IDeclaration DeclareIntrinsic(IModuleIntrinsic moduleIntrinsic, IScope parentScope) {
            var scope = new Parser.Scope {Parent = parentScope};
            var expressions = DeclareIntrinsics(moduleIntrinsic.Children, scope);
            var moduleDeclaration = new ModuleDeclaration {
                Name = moduleIntrinsic.Name,
                Scope = scope,
                Expressions = expressions
            };
            parentScope.Identifiers.Add(new ModuleEntry {ModuleDeclaration = moduleDeclaration});
            return moduleDeclaration;
        }

        IDeclaration DeclareIntrinsic(ITypeModuleIntrinsic typeIntrinsic, IScope parentScope) {
            var scope = new Parser.Scope {Parent = parentScope};
            var expressions = DeclareIntrinsics(typeIntrinsic.Children, scope);
            var moduleDeclaration = new IntrinsicModuleDeclaration {
                Name = typeIntrinsic.Name,
                Scope = scope,
                Expressions = expressions,
                FromLiteral = typeIntrinsic.FromLiteral,
                NetType = typeIntrinsic.NetType,
                ToNetType = typeIntrinsic.ToNetType,
                FromNetType = typeIntrinsic.FromNetType
            };
            AddTypeSizeDeclaration(typeIntrinsic.TypeSize, moduleDeclaration, parentScope);
            // TODO: add Construct/Destruct
            // TODO: add conversions

            if (typeIntrinsic.NetType != null) _netTypes[typeIntrinsic.NetType] = moduleDeclaration;
            parentScope.Identifiers.Add(new ModuleEntry {ModuleDeclaration = moduleDeclaration});
            return moduleDeclaration;
        }

        void AddTypeSizeDeclaration(ulong typeIntrinsicTypeSize, IModuleDeclaration moduleDeclaration, IScope parentScope) {
            var sizeType = moduleDeclaration.Name == SizeTypeName
                ? moduleDeclaration
                : (parentScope.Identifiers[SizeTypeName] as IModuleEntry)?.ModuleDeclaration;
            var sizeDefine = new VariableDeclaration {
                Name = "size",
                Type = sizeType,
                Value = new TypedValue {
                    Type = sizeType,
                    Data = BitConverter.GetBytes(typeIntrinsicTypeSize)
                }
            };
            var typeModule = new ModuleDeclaration {
                Name = "type",
                Scope = new Parser.Scope {Parent = moduleDeclaration.Scope},
                Expressions = {
                    sizeDefine
                }
            };
            typeModule.Scope.Identifiers.Add(new VariableEntry {Variable = sizeDefine});
            moduleDeclaration.Scope.Identifiers.Add(new ModuleEntry {ModuleDeclaration = typeModule});
            moduleDeclaration.Expressions.Add(typeModule);
        }

        IDeclaration DeclareIntrinsic(IFunctionIntrinsic functionIntrinsic, IScope parentScope) {
            var functionDeclaration = new FunctionDeclaration {
                Name = functionIntrinsic.Name,
                StaticScope = new Parser.Scope {Parent = parentScope},
                IsCompileTimeOnly = functionIntrinsic.IsCompileTimeOnly,
                Implementation = new ExpressionBlock {
                    Expressions = {
                        new IntrinsicExpression {
                            Intrinsic = functionIntrinsic
                        }
                    }
                }
            };
            parentScope.Identifiers.Add(
                new FunctionEntry {
                    FunctionDeclarations = {functionDeclaration}
                });
            return functionDeclaration;
        }

        NamedCollection<IArgumentDeclaration> TypeToArguments(Type type) {
            var result = new NamedCollection<IArgumentDeclaration>();
            if (type == null) return result;
            foreach (var field in type.GetRuntimeFields()) {
                result.Add(
                    new ArgumentDeclaration {
                        IsUnrolled = field.GetCustomAttributes(typeof(ArgumentUnrolled)).Any(),
                        IsAssignable = field.GetCustomAttributes(typeof(ArgumentAssignable)).Any(),
                        Name = field.Name,
                        Type = NetTypeToRebuildType(field.FieldType)
                        //Value = field.V
                    });
            }
            return result;
        }

        IModuleDeclaration NetTypeToRebuildType(Type type) {
            return _netTypes[type];
        }
    }
}
