using REC.AST;
using REC.Parser;
using REC.Tools;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using REC.Scope;
using REC.Instance;

namespace REC.Intrinsic
{
    using FieldToInstance = Dictionary<FieldInfo, ITypedInstance>;
    using NetTypes = Dictionary<Type, IModuleInstance>;

    class DeclarationConverter
    {
        readonly NetTypes _netTypes = new NetTypes();
        const string SizeTypeName = "u64";

        public static void BuildContext(IContext context, IIntrinsicDict intrinsicDict) {
            var converter = new DeclarationConverter();
            var declared = converter.DeclareIntrinsics(intrinsicDict, new ParentedIdentifierScope(context.Identifiers));
            converter.ConvertNetTypes(declared);
        }

        void ConvertNetTypes(IEnumerable<IInstance> declared) {
            foreach (var instance in declared) {
                if (instance is IntrinsicFunctionInstance function) {
                    var expression = function.IntrinsicExpression;
                    TypeToArguments(function.LeftArguments, function.Declaration.LeftArguments, function.ArgumentIdentifiers, ArgumentSide.Left, expression.Intrinsic.LeftArgumentsType, expression.LeftFields);
                    TypeToArguments(function.RightArguments, function.Declaration.RightArguments, function.ArgumentIdentifiers, ArgumentSide.Right, expression.Intrinsic.RightArgumentsType, expression.RightFields);
                    TypeToArguments(function.Results, function.Declaration.Results, function.ArgumentIdentifiers, ArgumentSide.Result, expression.Intrinsic.ResultType, expression.ResultFields);
                }
            }
        }


        IList<IInstance> DeclareIntrinsics(IEnumerable<IIntrinsic> intrinsics, IParentedIdentifierScope scope) {
            return intrinsics.Select(intrinsic => (IInstance) DeclareIntrinsic((dynamic) intrinsic, scope)).ToList();
        }

        IInstance DeclareIntrinsic(IModuleIntrinsic moduleIntrinsic, IParentedIdentifierScope scope) {
            var module = new ModuleInstance(moduleIntrinsic.Name);
            DeclareIntrinsics(moduleIntrinsic.Children, new ParentedIdentifierScope(module.Identifiers) {Parent = scope});

            scope.Add(module);
            return module;
        }

        IInstance DeclareIntrinsic(ITypeModuleIntrinsic typeIntrinsic, IParentedIdentifierScope scope) {
            var module = new IntrinsicTypeModuleInstance(typeIntrinsic);
            DeclareIntrinsics(typeIntrinsic.Children, new ParentedIdentifierScope(module.Identifiers) {Parent = scope});

            AddTypeSizeDeclaration(typeIntrinsic.TypeSize, module, scope);
            // TODO: add Construct/Destruct
            // TODO: add conversions

            if (typeIntrinsic.NetType != null) _netTypes[typeIntrinsic.NetType] = module;
            scope.Add(module);
            return module;
        }

        static void AddTypeSizeDeclaration(ulong typeIntrinsicTypeSize, IModuleInstance module, IParentedIdentifierScope scope) {
            var sizeType = module.Name == SizeTypeName
                ? module
                : (scope[SizeTypeName] as IModuleInstance);
            var sizeDefine = new VariableDeclaration {
                Name = "size",
                Type = sizeType,
                Value = new TypedValue {
                    Type = sizeType,
                    Data = BitConverter.GetBytes(typeIntrinsicTypeSize)
                }
            };
            var typeModule = new ModuleInstance(name: "type");
            typeModule.Identifiers.Add(new VariableInstance {Variable = sizeDefine});

            module.Identifiers.Add(typeModule);
        }

        IInstance DeclareIntrinsic(IFunctionIntrinsic functionIntrinsic, IParentedIdentifierScope scope) {
            var function = new IntrinsicFunctionInstance(functionIntrinsic);
            scope.Add(function);
            return function;
        }

        private void TypeToArguments(NamedCollection<IArgumentInstance> arguments, NamedCollection<IArgumentDeclaration> declarations, ILocalIdentifierScope identifiers, ArgumentSide side, Type type, FieldToInstance fields)
        {
            if (type == null)
                return;
            foreach (var field in type.GetRuntimeFields()) {
                var argumentDecl = new ArgumentDeclaration {
                    IsUnrolled = field.GetCustomAttributes(typeof(ArgumentUnrolled)).Any(),
                    IsAssignable = field.GetCustomAttributes(typeof(ArgumentAssignable)).Any(),
                    Name = field.Name,
                    Type = NetTypeToRebuildType(field.FieldType)
                    //Value = field.V
                };
                var argument = new ArgumentInstance {
                    Argument = argumentDecl,
                    Side = side
                };
                declarations.Add(argumentDecl);
                identifiers.Add(argument);
                arguments.Add(argument);
                fields.Add(field, argument);
            }
        }

        IModuleInstance NetTypeToRebuildType(Type type) {
            return _netTypes[type];
        }
    }
}
