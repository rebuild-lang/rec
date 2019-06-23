#include "execution/Machine.h"

#include "instance/Function.builder.h"
#include "instance/Function.ostream.h"
#include "instance/Scope.builder.h"
#include "instance/Type.builder.h"
#include "instance/TypeTree.builder.h"

#include "nesting/Token.builder.h"

#include "parser/Tree.builder.h"
#include "parser/Tree.ostream.h"

#include "gtest/gtest.h"

#include <memory>

namespace expression = parser;
namespace block = nesting;

struct ExecutionMachineData {
    const char* name{};
    std::shared_ptr<instance::Scope> scope;
    parser::Call call;
    std::shared_ptr<execution::Compiler> compiler;
    std::shared_ptr<execution::Context> context;

    strings::String expected;
    strings::String result;

    inline static ExecutionMachineData* instance;

    ExecutionMachineData(const char* name)
        : name{name}
        , scope{std::make_shared<instance::Scope>()}
        , compiler{std::make_shared<execution::Compiler>()}
        , context{std::make_shared<execution::Context>()} {
        context->compiler = compiler.get();
    }

    template<class... Instance>
    auto ctx(Instance&&... instance) && -> ExecutionMachineData {
        instance::buildScope(*scope, std::forward<Instance>(instance)...);
        return std::move(*this);
    }

    template<size_t N>
    auto expect(const char (&text)[N]) && -> ExecutionMachineData {
        expected = strings::String(text);
        return std::move(*this);
    }

    auto run(parser::details::CallBuilder&& callBuilder) && -> ExecutionMachineData {
        call = std::move(callBuilder).build(*scope);
        return std::move(*this);
    }

    static void literal(uint8_t* memory, intrinsic::Context*) {
        auto& lit = *reinterpret_cast<parser::NumberLiteral*>(memory);
        instance->result = strings::String{lit.value.integerPart};
    }
};

static auto operator<<(std::ostream& out, const ExecutionMachineData& emd) -> std::ostream& {
    out << "name: " << emd.name << "\n";
    out << "input:\n";
    out << emd.call << '\n';
    out << "expected:\n";
    out << emd.expected << '\n';
    return out;
}

class MachineTests : public testing::TestWithParam<ExecutionMachineData> {};

TEST_P(MachineTests, call) {
    const ExecutionMachineData& data = GetParam(); //
    const auto& call = data.call;
    const auto& context = *data.context;

    ExecutionMachineData::instance = const_cast<ExecutionMachineData*>(&data);
    execution::Machine::runCall(call, context);

    EXPECT_EQ(data.expected, data.result);
}

INSTANTIATE_TEST_CASE_P(
    simple,
    MachineTests,
    ::testing::Values( //
        ExecutionMachineData("Example")
            .ctx(
                instance::typeModT<nesting::NumberLiteral>("Lit"),
                instance::fun("print")
                    .params(instance::param("v").right().type(parser::type().instance("Lit")))
                    .rawIntrinsic(&ExecutionMachineData::literal))
            .run(parser::call("print").right(parser::arg("v", parser::expr(nesting::num("42")).typeName("Lit"))))
            .expect("42")));
