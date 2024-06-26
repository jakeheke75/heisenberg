#include "algo.hpp"
#include "cc_damped_mr.hpp"
#include "cc_kaufman.hpp"
#include "cc_simple_mr.hpp"

#include "ccex_order_executor.hpp"
#include "trade_stream_maker.hpp"
#include "utils.hpp"

#include "program_options.hpp"

#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <iostream>

namespace profitview
{

struct ProgramArgs
{
    std::string algo;
    std::string exchange;
    std::string api_key;
    std::string api_secret;
    std::string api_phrase;
    std::string sub_account;
    int lookback = 0;
    double reversion_level = 0.0;
    double base_quantity = 0.0;
    int er_period = 0, fast_sc = 0, slow_sc = 0, kama_trend = 0;
    double damping = 0.0;
    std::vector<std::string> symbols;

    void addOptions(boost::program_options::options_description& options)
    {
        namespace po = boost::program_options;
        // clang-format off
        options.add_options()
            ("algo", po::value(&algo)->required(), "Algo to use")
            ("exchange", po::value(&exchange)->required(), "Crypto Exchange to execute on.")
            ("api_key", po::value(&api_key)->required(), "API key for Cypto exchange.")
            ("api_secret", po::value(&api_secret)->required(), "API secret for Cypto exchange.")
            ("api_phrase", po::value(&api_phrase), "API phrase for Cypto exchange.")
            ("sub_account", po::value(&sub_account), "Subaccount on Cypto exchange.")
            ("lookback", po::value(&lookback)->required(), "Time period to look back")
            ("reversion_level", po::value(&reversion_level), "Mean reversion level.")
            ("base_quantity", po::value(&base_quantity)->required(), "Quantity to trade.")
            ("er_period", po::value(&er_period), "Efficiency Ratio base period for Kaufman")
            ("fast_sc", po::value(&fast_sc), "Fast exponential moving average smoothing period")
            ("slow_sc", po::value(&slow_sc), "Slow exponential moving average smoothing period")
            ("kama_trend", po::value(&kama_trend), "Kaufman trend prediction period")
            ("damping", po::value(&damping), "Standard deviation damping limit")
            ("symbols", po::value(&symbols)->multitoken()->required(), "Symbols for cypto assets to trade.");
        // clang-format on
    }
};

}    // namespace profitview

int main(int argc, char* argv[])
{
    using namespace profitview;
    ProgramArgs options;
    auto const result = parseProgramOptions(argc, argv, options);
    if (result)
        return result.value();

    const std::map<std::string, Algo> algos{
        {"SimpleMR", SimpleMR},
        {"Kaufman",  Kaufman },
        {"Damped",   Damped  }
    };

    CcexOrderExecutor executor{options.exchange, options.api_key, options.api_secret, options.api_phrase, options.sub_account};

    switch (algos.at(options.algo))
    {
    case SimpleMR:
        TradeStreamMaker::register_stream<CcSimpleMR<>>(
            options.algo, &executor, options.lookback, options.reversion_level, options.base_quantity);
        break;
    case Kaufman:
        TradeStreamMaker::register_stream<CcKaufman<>>(
            options.algo,
            &executor,
            options.lookback,
            options.base_quantity,
            options.er_period,
            options.fast_sc,
            options.slow_sc,
            options.kama_trend);
        break;
    case Damped:
        TradeStreamMaker::register_stream<CcDamped<>>(
            options.algo, &executor, options.lookback, options.reversion_level, options.base_quantity, options.damping);
        break;
    default: 
        BOOST_LOG_TRIVIAL(error) << "Unknown algo" << std::endl; return 2;
    }

    TradeStreamMaker::get(options.algo).subscribe(options.exchange, options.symbols);

    std::cout << "Press enter to quit" << std::endl;
    std::cin.get();

    enum
    {
        OrderId,
        Symbol,
        OrderSide,
        Size,
        Price,
        Time,
        Status
    };
    for (const auto& [cid, details] : executor.get_open_orders())
        BOOST_LOG_TRIVIAL(info) << "cid: " << cid << ", Order Id: " << std::get<OrderId>(details)
                  << ", Symbol: " << std::get<Symbol>(details) << ", Status: " << std::get<Status>(details)
                  << std::endl;

    return 0;
}