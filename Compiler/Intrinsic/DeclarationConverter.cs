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

        public static void BuildScope(IScope scope, IIntrinsicDict intrinsicDict) {
            var converter = new DeclarationConverter();
            var declared = converter.DeclareIntrinsics(intrinsicDict, scope);
            converter.ConvertNetTypes(declared, intrinsicDict);
        }

        void ConvertNetTypes(ICollection<IExpression> expressions, IIntrinsicDict intrinsicDict) {
            foreach (var expression in expressions) {
                ConvertNetTypes((dynamic)expression, intrinsicDict);
            }
        }

        void ConvertNetTypes(IModuleDeclaration moduleDeclaration, IIntrinsicDict intrinsicDict) {
            var intrinsic = intrinsicDict[moduleDeclaration.Name] as IModuleIntrinsic;
            if (intrinsic == null) return;
            ConvertNetTypes(moduleDeclaration.Expressions, intrinsic.Children);
            if (moduleDeclaration.IsType()) {
                var typeEntry = moduleDeclaration.Scope.Identifiers["type"] as IModuleEntry;
                var sizeEntry = typeEntry?.ModuleDeclaration.Scope.Identifiers["size"] as IVariableEntry;
                if (sizeEntry != null) {
                    ((VariableDeclaration) sizeEntry.Variable).Type = NetTypeToRebuildType(typeof(ulong));
                    ((TypedValue) sizeEntry.Variable.Value).Type = sizeEntry.Variable.Type;
                }
            }
        }

        void ConvertNetTypes(FunctionDeclaration functionDeclaration, IIntrinsicDict intrinsicDict) {
            var intrinsic = intrinsicDict[functionDeclaration.Name] as IFunctionIntrinsic;
            if (intrinsic == null) return;
            functionDeclaration.LeftArguments = TypeToArguments(intrinsic.LeftArgumentsType);
            functionDeclaration.RightArguments = TypeToArguments(intrinsic.RightArgumentsType);
            functionDeclaration.Results = TypeToArguments(intrinsic.ResultType);
        }

        IList<IExpression> DeclareIntrinsics(IEnumerable<IIntrinsic> intrinsics, IScope scope) {
            return intrinsics.Select(intrinsic => (IExpression) DeclareIntrinsic((dynamic) intrinsic, scope)).ToList();
        }

        IDeclaration DeclareIntrinsic(ModuleIntrinsic moduleIntrinsic, IScope parentScope) {
            var scope = new Parser.Scope { Parent = parentScope };
            var expressions = DeclareIntrinsics(moduleIntrinsic.Children, scope);
            var moduleDeclaration = new ModuleDeclaration {
                Name = moduleIntrinsic.Name,
                Scope = scope,
                Expressions = expressions
            };
            parentScope.Identifiers.Add(new ModuleEntry {ModuleDeclaration = moduleDeclaration});
            return moduleDeclaration;
        }

        IDeclaration DeclareIntrinsic(TypeModuleIntrinsic typeIntrinsic, IScope parentScope) {
            var scope = new Parser.Scope { Parent = parentScope };
            var expressions = DeclareIntrinsics(typeIntrinsic.Children, scope);
            var moduleDeclaration = new IntrinsicModuleDeclaration {
                Name = typeIntrinsic.Name,
                Scope = scope,
                Expressions = expressions,
                FromLiteral = typeIntrinsic.FromLiteral,
                NetType = typeIntrinsic.NetType,
                ToNetType = typeIntrinsic.ToNetType,
                FromNetType = typeIntrinsic.FromNetType,
            };
            AddTypeSizeDeclaration(typeIntrinsic.TypeSize, moduleDeclaration);
            // TODO: add Construct/Destruct
            // TODO: add conversions

            if (typeIntrinsic.NetType != null) {
                _netTypes[typeIntrinsic.NetType] = moduleDeclaration;
            }
            parentScope.Identifiers.Add(new ModuleEntry { ModuleDeclaration = moduleDeclaration });
            return moduleDeclaration;
        }

        void AddTypeSizeDeclaration(ulong typeIntrinsicTypeSize, ModuleDeclaration moduleDeclaration) {
            var sizeDefine = new VariableDeclaration {
                Name = "size",
                Value = new TypedValue {
                    Data = BitConverter.GetBytes(typeIntrinsicTypeSize)
                }
            };
            var typeModule = new ModuleDeclaration {
                Name = "type",
                Scope = new Parser.Scope { Parent = moduleDeclaration.Scope },
                Expressions = {
                    sizeDefine
                }
            };
            typeModule.Scope.Identifiers.Add(new VariableEntry { Variable = sizeDefine });
            moduleDeclaration.Scope.Identifiers.Add(new ModuleEntry {ModuleDeclaration = typeModule});
            moduleDeclaration.Expressions.Add(typeModule);
        }

        IDeclaration DeclareIntrinsic(IFunctionIntrinsic functionIntrinsic, IScope parentScope) {
            var functionDeclaration = new FunctionDeclaration {
                Name = functionIntrinsic.Name,
                StaticScope = new Parser.Scope { Parent = parentScope},
                IsCompileTimeOnly = functionIntrinsic.IsCompileTimeOnly,
                Implementation = new ExpressionBlock {
                    Expressions = {
                        new IntrinsicExpression {
                            Intrinsic = functionIntrinsic
                        }
                    }
                }
            };
            parentScope.Identifiers.Add(new FunctionEnty {
                FunctionDeclarations = { functionDeclaration }
            });
            return functionDeclaration;
        }

        NamedCollection<IArgumentDeclaration> TypeToArguments(Type type) {
            var result = new NamedCollection<IArgumentDeclaration>();
            if (type == null) return result;
            foreach (var field in type.GetRuntimeFields()) {
                result.Add(new ArgumentDeclaration {
                    IsUnrolled = field.GetCustomAttributes(typeof(ArgumentUnrolled)).Any(),
                    Name = field.Name,
                    Type = NetTypeToRebuildType(field.FieldType),
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