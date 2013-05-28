#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <functional>

namespace com {
namespace ubuntu {
namespace location {

struct ProgramOptions
{
    ProgramOptions(bool do_allow_unregistered = true) : allow_unregistered(do_allow_unregistered)
    {
    }

    ProgramOptions& add(const char* name, const char* desc)
    {
        od.add_options()
                (name, desc);
        return *this;
    }

    template<typename T>
    ProgramOptions& add(const char* name, const char* desc)
    {
        od.add_options()
                (name, boost::program_options::value<T>(), desc);
        return *this;
    }
    
    template<typename T>
    ProgramOptions& add(const char* name, const char* desc, const T& default_value)
    {
        od.add_options()
                (name, boost::program_options::value<T>()->default_value(default_value), desc);
        return *this;
    }

    template<typename T>
    ProgramOptions& add_composed(const char* name, const char* desc)
    {
        od.add_options()(
            name, 
            boost::program_options::value<T>()->composing(),
            desc);
        return *this;
    }

    bool parse_from_command_line_args(int argc, char** argv)
    {
        try
        {
            auto parser = boost::program_options::command_line_parser(
                argc, 
                argv).options(od);
            auto parsed = allow_unregistered ? parser.allow_unregistered().run() : parser.run();
            unrecognized = boost::program_options::collect_unrecognized(
                parsed.options, 
                boost::program_options::exclude_positional);
            boost::program_options::store(parsed, vm);
        } catch(const std::runtime_error& e)
        {
            std::cerr << e.what() << std::endl;
            return false;
        }

        return true;
    }

    template<typename T>
    T value_for_key(const std::string& key)
    {
        return vm[key].as<T>();
    }

    std::size_t value_count_for_key(const std::string& key)
    {
        return vm.count(key);
    }

    void enumerate_unrecognized_options(const std::function<void(const std::string&)>& enumerator)
    {
        for (const std::string& s : unrecognized)
            enumerator(s);
    }

    void print_help(std::ostream& out)
    {
        out << od;
    }

    bool allow_unregistered;
    boost::program_options::options_description od;
    boost::program_options::variables_map vm;
    std::vector<std::string> unrecognized;
};

}
}
}
