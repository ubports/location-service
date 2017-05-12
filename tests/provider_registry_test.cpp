#include <location/provider_registry.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

namespace cul = location;

namespace
{
struct Factory
{
    MOCK_METHOD1(create, cul::Provider::Ptr(const location::util::settings::Source&));
};
}

TEST(ProviderRegistry, adding_a_factory_works)
{    
    const std::string provider_name{"ATotallyDummyProviderRegistry"};
    Factory factory;
    auto factory_function = std::bind(&Factory::create, std::ref(factory), std::placeholders::_1);
    cul::ProviderRegistry::instance().add_provider_for_name(provider_name, factory_function);

    bool found = false;
    cul::ProviderRegistry::instance().enumerate([&found, &provider_name](const std::string& name, const cul::ProviderRegistry::Factory&, const cul::ProviderRegistry::Options&)
    {
        if (!found)
            found = (provider_name == name);
    });

    EXPECT_TRUE(found);
}

TEST(ProviderRegistry, creating_for_known_name_invokes_factory_function)
{
    using namespace ::testing;

    const std::string provider_name{"ATotallyDummyProviderRegistry"};
    Factory factory;
    ON_CALL(factory, create(_)).WillByDefault(Return(cul::Provider::Ptr{}));
    EXPECT_CALL(factory, create(_)).Times(Exactly(1));
    auto factory_function = std::bind(&Factory::create, std::ref(factory), std::placeholders::_1);
    cul::ProviderRegistry::instance().add_provider_for_name(provider_name, factory_function);

    cul::ProviderRegistry::instance().create_provider_for_name_with_config(provider_name, location::util::settings::Source{});
}

TEST(ProviderRegistry, attempt_to_create_for_unknown_name_returns_null_ptr)
{
    EXPECT_EQ(cul::Provider::Ptr{},
              cul::ProviderRegistry::instance().create_provider_for_name_with_config(
                  "AnUnknownProvider", 
                  location::util::settings::Source{}));
}
