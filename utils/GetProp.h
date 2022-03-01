#ifndef _GetProp_h__
#define _GetProp_h__

#include "Exceptions.h"

#include <string>       // std::string
#include <map>          // std::map
#include <vector>       // std::vector
#include <iterator>     // std::iterator


#define RCSID_GetProp_h "$Id: GetProp.h $"
extern const std::string RCSID_GetProp_cc;

class GetProp
{
    protected:
        std::string config_file;

    public:

        // init and constructors
        GetProp(){};
        static GetProp *getInstance();
        void init(std::string file) throw(Error);
        void init(std::string file, int argc, char* argv[]) throw(Error);

        // collection methods
    	std::string getConfigFile() const { return config_file; }
        int getPropertyCount() const;
        std::vector<std::string> getConfigListNames();  // list of properties in config file
        bool contains(const std::string name);
        std::string toString(std::string prefix = "");

        // get property values
        std::string resolveName(const std::string name);
        std::string getString(const std::string name) throw(Error);
        int    getInt(const std::string name) throw(Error);
        double getDouble(const std::string name) throw(Error);
        bool   getBool(const std::string name) throw(Error);
        void   getDoubleArray(const std::string name, int num, double *values) throw(Error);

        template<typename T> std::vector<T> getVector(const std::string name) throw(Error);
        template<typename T> std::vector<T> toVector(std::string) throw(Error);
        void fillVector(std::vector<int>*, std::string);
        void fillVector(std::vector<double>*, std::string);
        void fillVector(std::vector<std::string>*, std::string);

        // utility functions
        static int toInt(std::string) throw(Error);
        static double toDouble(std::string) throw(Error);
        static bool toBool(std::string) throw(Error);
        static void toDoubleArray(std::string s, int num, double *values) throw(Error);

    protected:
        std::map<std::string, std::string> kvm;         // list in config file
        std::map<std::string, std::string> pkvm;        // predefined key value map

    public:
        // unit tests
        static void Internal_unit_tests();
        static void File_unit_tests(GetProp* prop);
        static int main(int argc, char* argv[]);

}; // class GetProp

template<typename T> 
std::ostream& operator<<(std::ostream &os, std::vector<T> vec){
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<T>(os, " "));
    return os;
};

#endif
// end of file: GetProp.h
