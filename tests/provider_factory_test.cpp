#include <com/ubuntu/location/provider_factory.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>

namespace cul = com::ubuntu::location;

namespace
{
struct Factory
{
    MOCK_METHOD1(create, cul::Provider::Ptr(const cul::ProviderFactory::Configuration&));
};
}

TEST(ProviderFactory, adding_a_factory_works)
{    
    const std::string provider_name{"ATotallyDummyProviderFactory"};
    Factory factory;
    auto factory_function = std::bind(&Factory::create, std::ref(factory), std::placeholders::_1);
    cul::ProviderFactory::instance().add_factory_for_name(provider_name, factory_function);

    bool found = false;
    cul::ProviderFactory::instance().enumerate([&found, &provider_name](const std::string& name, const cul::ProviderFactory::Factory&)
                                               {
                                                   if (!found)
                                                       found = (provider_name == name);
                                               });

    EXPECT_TRUE(found);
}

TEST(ProviderFactory, creating_for_known_name_invokes_factory_function)
{
    using namespace ::testing;

    const std::string provider_name{"ATotallyDummyProviderFactory"};
    Factory factory;
    ON_CALL(factory, create(_)).WillByDefault(Return(cul::Provider::Ptr{}));
    EXPECT_CALL(factory, create(_)).Times(Exactly(1));
    auto factory_function = std::bind(&Factory::create, std::ref(factory), std::placeholders::_1);
    cul::ProviderFactory::instance().add_factory_for_name(provider_name, factory_function);

    cul::ProviderFactory::instance().create_provider_for_name_with_config(provider_name, cul::ProviderFactory::Configuration{});
}

TEST(ProviderFactory, attempt_to_create_for_unknown_name_returns_null_ptr)
{
    EXPECT_EQ(cul::Provider::Ptr{},
              cul::ProviderFactory::instance().create_provider_for_name_with_config(
                  "AnUnknownProvider", 
                  cul::ProviderFactory::Configuration{}));
}

TEST(ProviderFactory, extracting_undecorated_provider_name_works)
{
    EXPECT_EQ("remote::Provider", cul::ProviderFactory::extract_undecorated_name("remote::Provider@gps"));
}
