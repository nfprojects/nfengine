/**
 * @file
 * @author Witek902 (witek902@gmail.com)
 * @brief  Performance tests for Config class.
 */

#include "PCH.hpp"
#include "nfCommon/Config/Config.hpp"
#include "nfCommon/Config/ConfigValue.hpp"
#include "nfCommon/System/Timer.hpp"

using namespace NFE::Common;

TEST(Config, GenerateAndParse)
{
    const int MAX_DEPTH = 12;
    Timer timer;

    std::cout << "Depth | Build time | Generate time | Parse time | String size (chars)"
              << std::endl;

    for (int maxDepth = 2; maxDepth < MAX_DEPTH; ++maxDepth)
    {
        double buildTime, generateTime;
        String configString;

        {
            Config config;

            // explicit lambda type definition is required for recursive lambdas
            typedef std::function<ConfigObject(int)> LambdaType;

            LambdaType generateNode = [&](int depth)
            {
                ConfigObject node;
                config.AddValue(node, StringView("a"), ConfigValue(1));
                config.AddValue(node, StringView("b"), ConfigValue(true));
                config.AddValue(node, StringView("c"), ConfigValue(1.2f));
                config.AddValue(node, StringView("d"), ConfigValue("a"));

                ConfigArray array;
                config.AddValue(array, ConfigValue(1));
                config.AddValue(array, ConfigValue(2));
                config.AddValue(node, StringView("e"), ConfigValue(array));

                if (depth < maxDepth)
                {
                    config.AddValue(node, StringView("f"), ConfigValue(generateNode(depth + 1)));
                    config.AddValue(node, StringView("g"), ConfigValue(generateNode(depth + 1)));
                }

                return node;
            };

            // measure config tree build time
            timer.Start();
            config.SetRoot(generateNode(0));
            buildTime = 1000.0 * timer.Stop();

            // measure string generation time
            timer.Start();
            configString = config.ToString();
            generateTime = 1000.0 * timer.Stop();
        }

        // measure parsing time
        timer.Start();
        {
            Config config;
            config.Parse(configString.Str());
        }
        double parseTime = 1000.0 * timer.Stop();

        // print stats
        std::cout << std::setprecision(5) << std::left
            << std::setw(5) << maxDepth << " | "
            << std::setw(10) << buildTime << " | "
            << std::setw(13) << generateTime << " | "
            << std::setw(10) << parseTime << " | "
            << configString.Length() << std::endl;
    }
}
