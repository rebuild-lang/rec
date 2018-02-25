#include "execution/Machine.h"

#include "block/TokenBuilder.h"
#include "expression/TreeBuilder.h"
#include "expression/TreeOutput.h"
#include "instance/FunctionBuilder.h"
#include "instance/FunctionOutput.h"
#include "instance/ScopeBuilder.h"
#include "instance/TypeBuilder.h"

#include "gtest/gtest.h"

#include <memory>

namespace expression = parser::expression;
namespace block = parser::block;

struct ExecutionMachineData {
    const char* name{};
    std::shared_ptr<instance::Scope> scope;
    expression::Invocation invocation;
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

    auto invoke(expression::details::InvokeBuilder&& invoke) && -> ExecutionMachineData {
        invocation = std::move(invoke).build(*scope);
        return std::move(*this);
    }

    static void literal(uint8_t* memory) {
        auto& lit = reinterpret_cast<expression::NumberLiteral*&>(*memory);
        instance->result = strings::String{lit->integerPart};
    }
};

static auto operator<<(std::ostream& out, const ExecutionMachineData& emd) -> std::ostream& {
    out << "name: " << emd.name << "\n";
    out << "input:\n";
    out << emd.invocation << '\n';
    out << "expected:\n";
    out << emd.expected << '\n';
    return out;
}

class MachineTests : public testing::TestWithParam<ExecutionMachineData> {};

TEST_P(MachineTests, invoke) {
    const ExecutionMachineData& data = GetParam(); //
    const auto& invocation = data.invocation;
    const auto& context = *data.context;

    ExecutionMachineData::instance = const_cast<ExecutionMachineData*>(&data);
    execution::Machine::invoke(invocation, context);

    EXPECT_EQ(data.expected, data.result);
}

INSTANTIATE_TEST_CASE_P(
    simple,
    MachineTests,
    ::testing::Values( //
        ExecutionMachineData("Example")
            .ctx(
                instance::typeMod("Lit").size(sizeof(void*)).build(),
                instance::fun("print")
                    .args(instance::arg("v").right().type("Lit"))
                    .rawIntrinsic(&ExecutionMachineData::literal))
            .invoke(
                expression::invoke("print").right(expression::arg("v", expression::LiteralVariant{block::num("42")})))
            .expect("42")));
