#!/usr/bin/python
import os
import sys
import time

Version = 'Id propertiesGen_new.py 0000 2019-04-30 01:33:00 claude'

typ2typ = {'s': 'std::string', 'i': 'int', 'f': 'double', 'd': 'double', 'b': 'bool', 'dv': 'std::vector<double>', 'iv': 'std::vector<int>', 'sv': 'std::vector<std::string>'}
meta2typ = {'string': 's', 'int': 'i', 'float': 'f', 'double': 'd', 'bool': 'b', 'double[]': 'dv', 'int[]': 'iv', 'string[]': 'sv'}

def readmeta(infile, cmnclasses) :
    myvars = {}
    group = 'parent'
    fin = open(infile,'r')
    idtag = ''
    for l in fin :
        minvar = ''
        maxvar = ''
        if l.startswith('#') or len(l.split()) == 0 : # allow comment lines
            continue
        if l.startswith('"$Id:') :
            # expanded Id tag
            idtag = l
            continue
        if l.startswith('"$Id') :
            # unexpanded Id tag, continue
            continue
        if l.startswith('group:') :
            group = l.split()[1]
            continue
        if l.startswith('endgroup') :
            group = 'parent'
            continue 
        if l.startswith('@include') :
            meta = l.split()[1] 
            metaclass =  meta.split(os.sep)[-1].replace('.meta','')
            metaqual = l.split()[2:]
            cmnclasses[metaclass] = []
            if len(metaqual) > 0 :
                for mq in metaqual :
                    cmnclasses[metaclass].append(mq)
            else :
                mq = 'my' + metaclass.lower()
                cmnclasses[metaclass].append(mq)
            continue
        fs = l.split()
        if fs[1] in meta2typ.keys() :
            typ = meta2typ[fs[1]]
        else :
            print 'ERROR: invalid type specification for', fs[0]
            return
        if typ in ['i','f','d'] and len(l[:l.find('#')].split()) > 4 :
            minvar = fs[3]
            maxvar = fs[4]
        if l.find('#') > -1 :
            comment = l[l.find('#'):].replace('#','').rstrip().lstrip()
        else :
            comment = ''
        if fs[0] in myvars :
            print '\n***********\nWARNING: VARIABLE', fs[0], 'ALREADY IN MAP!!\nDuplicate variable names are not supported and the latest entry will be used\n***********\n'
        myvars[fs[0]] = ['_%s' % fs[0].lower().replace('.','_'), fs[2], typ, minvar, maxvar, comment, group]
    fin.close()

    return myvars, cmnclasses, idtag


def writecc_varmap(ofile, vars, root, cmnclasses) :
    mandatory = [v for v in vars if len([x for x in vars[v] if x.find("REQUIRED") > -1]) > 0]
    sensitive = [v for v in vars if len([x for x in vars[v] if x.find("SENSITIVE") > -1]) > 0]

    ## Init function to read all values from GetProp's kvm structure into local vars 
    str = 'void %s::init() throw(Error) {\n' % root
    str += '\tmissing = 0;\n'
    str += '\tdups = 0;\n'
    if len(cmnclasses) > 0 :
        for ccn in [c for c in cmnclasses] :
            nmq = len(cmnclasses[ccn])
            if nmq > 1 or (nmq == 1 and cmnclasses[ccn][0] != 'my'+ccn.lower()):
                for mq in cmnclasses[ccn] :
                    str += '\t%s = %s(*this, std::string(\"%s\"));\n' % (mq, ccn, mq)
            else :
                str += '\tif (m_sPrefix.length() > 0) {\n'
                str += '\t\t%s = %s(*this, m_sPrefix.substr(0,m_sPrefix.size()-1));\n' % (cmnclasses[ccn][0], ccn)
                str += '\t} else {\n'
                str += '\t\t%s = %s(*this);\n' % (cmnclasses[ccn][0], ccn)
                str += '\t}\n'
            for mq in cmnclasses[ccn] :
                str += '\t%s.init();\n' % mq
                str += '\tmissing += %s.getMissing();\n' % mq
    str += '\tstd::ostringstream ostrm;\n\n'
    str += '\tostrm << \"CONFIG: %s Config file: \" << getConfigFile() << \" (prefix: \" << m_sPrefix.substr(0,m_sPrefix.size()-1) << \")\" << std::endl;\n\n' % root
    ofile.write(str)

    gps = ['parent']
    gps.extend(set([vars[h][-1] for h in vars if vars[h][-1] != 'parent']))
    for g in gps :
        for param in sorted([v for v in vars if vars[v][-1] == g]) :
            var = vars[param][0]
            default = vars[param][1].replace('|SENSITIVE','')
            typ = vars[param][2]
            min = vars[param][3]
            max = vars[param][4]
            default = default.replace('"','').replace("'","")
            if len(default) == 0 :
                defaultstr = "''"
            else :
                defaultstr = default
            if param in sensitive :
                if len(default) > 0 :
                    if typ == 's' :
                        defaultstr = default[0] + '...' + default[-1]
                    else :
                        defaultstr = '...'
                comment = ' and sensitive'
                varstr = 'temp'
            else :
                comment = ''
                varstr = var
            str = '\tif ( !contains(m_sPrefix + \"%s\") ) {\n' % param
            if param in mandatory :
                str += '\t\tthrow Error(getConfigFile() + \": Parameter %s not found.\");\n\t}' % (param)
            else :
                if typ == 's' :
                    str += '\t\t%s = \"%s\";' % (var,default.replace('"','').replace("'",""))
                elif typ in ['dv', 'iv'] :
                    for x in default.split(',') :
                        str += '\t\t%s.push_back(%s);\n' % (var, x)
                elif typ == 'sv' :
                    for x in default.split(',') :
                        str += '\t\t%s.push_back(\"%s\");\n' % (var, x)
                else :
                    str += '\t\t%s = %s;' % (var,default)
                str += '\n\t\tostrm << \"CONFIG:   %s: %s using default value%s\" << std::endl;' % (param, defaultstr, comment.replace(' and',', field is'))
                str += '\n\t\tmissing++;\n'
                str += '\t}' 
            str += '\n\telse {\n\t\ttry {\n\t\t\t%s = ' % var
            if typ == 's' :
                str += 'getString(m_sPrefix + \"%s\");' % param
            elif typ == 'i' :
                str += 'getInt(m_sPrefix + \"%s\");' % param
            elif typ == 'f' :
                str += '(float)getDouble(m_sPrefix + \"%s\");' % param
            elif typ == 'd' :
                str += 'getDouble(m_sPrefix + \"%s\");' % param
            elif typ == 'b' :
                str += 'getBool(m_sPrefix + \"%s\");' % param
            elif typ in ['dv', 'iv', 'sv'] :
                mytyp = typ2typ[typ].replace('std::vector<','').replace('>','')
                str += 'getVector<%s>(m_sPrefix + \"%s\");' % (mytyp, param)
            else :
                print 'ERROR: invalid type specification for', param
                return
            if param in sensitive :
                if typ == 's' :
                    str += '\n\t\t\tstd::string %s = %s.substr(0,1) + "..." + %s.substr(%s.length()-1,1);' % (varstr, var, var, var)
                else :
                    str += '\n\t\t\tstd::string %s = "...";' % (varstr)
            if param in mandatory :
                str += '\n\t\t\tostrm << \"CONFIG:   %s: \" << %s << \" (field is mandatory%s)\" << std::endl;' % (param, varstr, comment)
            else :
                str += '\n\t\t\tostrm << \"CONFIG:   %s: \" << %s << \" (default is %s)\" << std::endl;' % (param, varstr, defaultstr.replace("+\"/\"+"," + ") + comment.replace(' and',', field is'))
            str += '\n\t\t} catch(Error& e) {\n\t\t\tthrow Error(getConfigFile() + ": Parameter %s has wrong format!\");\n\t\t}\n' % (param)
            if min > '' and max > '' :
                str += '\t\tif (%s < %s || %s > %s)\n' % (var, min, var, max)
                str += '\t\t\tthrow Error(getConfigFile() + ": Parameter %s (" + getString(m_sPrefix + "%s") + ") is not between %s and %s");\n' % (param, param, min, max)
            str += '\t}\n\n'
            ofile.write(str)


    str = ''
    for param in vars :
        str += '\tmetalist.insert(m_sPrefix + \"%s\");\n' % param
    str += '\n'
    str += '\tLOGI << ostrm.str();\n\n'
    str += '} // %s::init\n\n' % root
    ofile.write(str)

    ## sethandle() function to set handle
    str = 'void %s::sethandle(%s* pHandle) {\n' % (root, root)
    str += '\thandle = pHandle;\n'
    str += '} // %s::sethandle\n\n' % root
    ofile.write(str)

    ## Check() function to compare expected and provided variables
    str = '/** function compares expected and provided (by configuration file) variables. Provides runtime warnings for missing (default used) and unused variables.*/\n' # with doxygen annotation
    str += 'void %s::checkVars() {\n' % root
    str += '\tstd::ostringstream ostrm;\n'
    str += '\tint unused = 0;\n\n'

    str += '\tstd::vector<std::string> configList = getConfigListNames(); // Get all properties from config\n'
    str += '\tstd::vector<std::string> propList = getPropListNames(); // Get all properties from config\n'
    str += '\tfor (std::vector<std::string>::iterator it = configList.begin(); it != configList.end(); it++) {\n'
    str += '\t\tif (std::find(propList.begin(), propList.end(), *it) == propList.end()) {\n'
    str += '\t\t\tostrm << \"WARNING: config variable \" << *it << \" (" << getString(*it) << \") is unused!\" << std::endl;\n'
    str += '\t\t\tunused++;\n'
    str += '\t\t}\n'
    str += '\t}\n'
    str += '\tif (unused > 0) {\n'
    str += '\t\tLOGW << ostrm.str();\n'
    str += '\t}\n'
    str += '\tLOGI << "CONFIG: Parsed " << getConfigFile() << ". Expected:" << propList.size() << ", read:" << configList.size() << ", default:" << missing << ", unused:" << unused << ", duplicates:" << dups << std::endl;\n'
    str += '} // %s::checkVars\n\n' % root
    ofile.write(str)

    return 


def writecc_getfns(ofile, vars, root) :
    for param in sorted([v for v in vars if v.find('.') == -1]) :
        var = vars[param][0]
        typ = vars[param][2]
        comment = vars[param][5]
        str = '/** function auto-generated and returns %s variable from configuration file parameters. Parameter: %s\n*@return %s */\n' % (param, comment, var) # doxygen annotation
        if typ in typ2typ.keys() :
            str += typ2typ[typ]
            str += ' ' 
        else :
            print 'ERROR: invalid type specification for', param
            return
        str += '%s::get%s() const {\n\treturn %s;\n}\n\n' % (root, param, var)
        ofile.write(str)

    for param in sorted(set([v.split('.')[1] for v in vars if v.find('.') > -1])) :
        mapparam = sorted([v for v in vars if v.find('.') > -1 and v.split('.')[1] == param])
        var = vars[mapparam[0]][0]
        typ = vars[mapparam[0]][2]
        comment = vars[mapparam[0]][5]
        str = '/** function auto-generated and returns %s variable from configuration file parameters. Parameter: %s\n*@return %s */\n' % (param, comment, var) # doxygen annotation 
        if typ in typ2typ.keys() :
            str += typ2typ[typ]
            str += ' ' 
        else :
            print 'ERROR: invalid type specification for', param
            return
        str += '%s::get%s(std::string qual) const throw(Error) {\n' % (root, param)
        for mp in mapparam :
            var = vars[mp][0]
            str += '\tif (qual == \"%s\") return %s;\n' % (mp.split('.')[0], var)
        str += '\tthrow Error(\"Calling %s::get%s() with invalid qualifier (undefined in meta file)!\");\n' % (root, param)
        str += '}\n\n'
        ofile.write(str)

    return


def writecc_unittestfn(ofile, vars, root, cmnclasses) :
    
    str = '\n\nstd::string %s::unittest() {\n' % root
    str += '\tstd::ostringstream ostrm;\n'
    str += '\tostrm << std::endl << "Exercise generated get methods for class: %s (prefix: " << m_sPrefix << ")" << std::endl;\n' % root
    for param in [v for v in vars if v.find('.') == -1] :
        str += '\tostrm << "  get%s() => " << get%s() << std::endl;\n' % (param, param)
    for param in [v for v in vars if v.find('.') > -1] :
        str += '\tostrm << "  get%s(%s) => " << get%s(\"%s\") << std::endl;\n' % (param.split('.')[1], param.split('.')[0], param.split('.')[1], param.split('.')[0])
    # DON'T KNOW HOW TO CHECK FOR CMNCLASS VARS!
    for cc in cmnclasses :
        for mq in cmnclasses[cc] :
            str += '\t%s.unittest();\n' % mq
    str += '\treturn ostrm.str();\n}\n\n'
    ofile.write(str);

    return
        

def writecc_unittest(ofile, vars, root, cmnclasses) :
    str = '\n'
    str += 'int %s::main(int argc, char* argv[]) {\n' % root
    str += '\n'
    str += '\tint rc = 0;       // return code\n'
    str += '\n'
    str += '\t// logging stuff\n'
    str += '\tstatic pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;\n'
    str += '\tstatic eew::ExternalMutexConsoleAppender<eew::SeverityFormatter> Severity_appender(&print_lock);\n'
    str += '\n'
    str += '\tplog::init(plog::verbose, &Severity_appender);\n'
    str += '\n'
    str += '\tLOGN << "Program: " << argv[0] << " -- autogenerated unit test for %s" << std::endl;\n' % root
    str += '\tLOGN << "Modules:";\n'
    str += '\tLOGN << "  " << RCSID_Exceptions_h;\n'
    str += '\tLOGN << "  " << RCSID_Exceptions_cc;\n'
    str += '\tLOGN << "  " << RCSID_GetProp_h;\n'
    str += '\tLOGN << "  " << RCSID_GetProp_cc;\n'
    str += '\tLOGN << "  " << RCSID_%s_h;\n' % root
    str += '\tLOGN << "  " << RCSID_%s_cc;\n' % root
    str += '\tLOGN;\n'
    str += '\n'
    str += '\tLOGI << "Begin unit test for %s";\n' % root
    str += '\n'
    str += '\tif (argc < 2) {\n'
    str += '\t\tLOGI << "Usage: " << argv[0] << " <config file>" << std::endl;\n'
    str += '\n'
    str += '\t\tLOGI << "No file specified";\n'
    str += '\t\ttry {\n'
    str += '\t\t\tLOGI << "Trying GetProp::Internal_unit_tests...";\n'
    str += '\t\t\tGetProp::Internal_unit_tests();\n'
    str += '\t\t} catch (Error& e) {\n'
    str += '\t\t\trc++;\n'
    str += '\t\t\tLOGE << "Error using GetProp file unit tests." << std::endl << e.str();\n'
    str += '\t\t}\n'
    str += '\t\tLOGI << std::endl << "End unit test for %s. returning " << rc;\n' % root
    str += '\t\texit(rc);\n'
    str += '\t}\n'
    str += '\n'
    str += '\t%s* prop = %s::getInstance();\n' % (root, root)
    str += '\ttry {\n'
    str += '\t\tstd::string filename = argv[1];\n'
    str += '\t\t// LOGD << "Attempting to open file " << filename;\n'
    str += '\t\tprop->init(filename, argc, argv);\n'
    str += '\n'
    str += '\t\tLOGI << "Trying GetProp::File_unit_tests...";\n'
    str += '\t\tGetProp::File_unit_tests(prop);\n'
    str += '\n'
    str += '\t} catch (Error& e) {\n'
    str += '\t\trc++;\n'
    str += '\t\tLOGE << "Error using GetProp file unit tests." << std::endl << e.str();\n'
    str += '\t\texit(rc);\n'
    str += '\t}\n'
    str += '\n'
    str += '\tLOGI << std::endl << "Trying %s specific unit tests...";\n' % root
    str += '\ttry {\n'
    str += '\t\tLOGI << prop->unittest();\n'
    str += '\t} catch (Error& e) {\n'
    str += '\t\trc++;\n'
    str += '\t\tLOGE << "Unexpected error using generated methods." << std::endl << e.str() << std::endl;\n'
    str += '\t}\n'
    str += '\n'
    str += '\tLOGI << std::endl << "End unit test of %s. returning " << rc;\n' % root
    str += '\n'
    str += '\texit(rc);\n'
    str += '} // static %s::main\n' % root

    ofile.write(str)

    return 


def writeh_vars(ofile, vars) :
    for param in sorted(vars) :
        var = vars[param][0]
        typ = vars[param][2]
        comment = vars[param][5]
        if typ in typ2typ.keys() :
            type = typ2typ[typ]
        else :
            print 'ERROR: invalid type specification for', param
            return
        str = '\t\t%s %s /**< %s */' % ('{0: <6}'.format(type), '{0: <30}'.format(var +";"), comment) # with doxygen annotation
        str += '\n'

        ofile.write(str)

    return


def writeh_getfns(ofile, vars) :
    seen = []
    for param in vars :
        if param.find('.') > -1 and param.split('.')[1] in seen :
            continue
        typ = vars[param][2]
        if typ in typ2typ.keys() :
            str = '\t\t' + typ2typ[typ] + ' ' 
        else :
            print 'ERROR: invalid type specification for', param
            return

        if param.find('.') > -1 :
            str += 'get%s(std::string qual) const throw(Error);\n' % param.split('.')[1]
            seen.append(param.split('.')[1])
        else :
            str += 'get%s() const;\n' % param
        ofile.write(str)

    return


def doxheadercc(root) :
    str = '/** @file %s.cc This auto-generated file handles configuration file reading and checking, variable initialisation (parameter setting) and variable get functions for the %s class.*/\n' % (root, root) # doxygen
    str += '/** @class %s \n * @brief class is auto-generated and handles configuration file parameters for the %s class and is derived from the %s.meta file.*/\n\n' % (root, root.replace('Properties',''), root) # doxygen
    return str


def doxheaderh(root) :
    str = '/** @file %s.h \n * @brief This auto-generated file consists of header information for the %s class.*/\n\n' % (root, root) # doxygen
    return str


def genheader(root) :
    str =  '/*\n'
    str += ' * THIS IS AN AUTOGENERATED FILE.  DO NOT MODIFY BY HAND!\n'
    str += ' * Created by %s\n' % Version
    str += ' * Generated %s\n' % time.strftime("%Y-%m-%d %H:%M:%S")
    str += ' * Edit %s.meta and rerun as follows:\n' % root
    str += ' *     %s %s\n' % (sys.argv[0], sys.argv[1])
    str += ' */\n\n'
    return str


def writeh(fname, vars, root, cmnclasses, idtag = '') :
    ofile = open(fname, 'w')

    # Write file headers
    str  = genheader(root)
    str += doxheaderh(root)
    str += '#ifndef __%s_h\n' % root
    str += '#define __%s_h\n' % root
    str += "\n"
    str += '#include <string>           // std::string\n'
    str += '#include <set>              // std::set\n'
    str += "\n"
    if len(cmnclasses) == 0 :
	    str += '#include \"GetProp.h\"        // GetProp\n'
    else :
        for cc in cmnclasses :
            str += '#include \"%s.h\"\n' % cc
    str += '\n'
    ofile.write(str)

    # Write standard 
    str = 'class %s : public GetProp {\n' % root
    str += '\n\tprotected:\n'
    str += '\t\tstatic %s *handle; /**< pointer handle to class instance if using getInstance function*/\n' % root # with doxygen annotation
    str += '\t\tstd::string m_sPrefix; /**< qualifier string used to differentiate multiple instances of a given included properties class. In the meta file, included classes may be given qualifer names if more than one instance is needed. In this case, variables for the included properties classes are defined in the configuration file using qualifier.variable. The m_sPrefix variable is used to hold the prefix (qualifier.) information to the variable name as required.*/\n' # with doxygen annotation
    str += '\t\tstd::set<std::string> metalist; /**< complete list of class variables defined in meta file*/\n' # with doxygen annotation
    str += '\t\tint missing; /**< number of variables not read from configuration file (therefore default will be used)*/\n\n' # with doxygen annotation
    str += '\t\tint dups; /**< number of variables in all properties classes that are duplicated, and will therefore be assigned the same config file value*/\n\n' # with doxygen annotation
    str += '\t\tvoid checkVars();\n\n'
    ofile.write(str)

    # Write included class instances
    str = '\t\t// included properties classes\n'
    for cc in cmnclasses :
        for mq in cmnclasses[cc] :
            str += '\t\t%s %s;\n' % (cc, mq) 
    str += '\n'
    ofile.write(str)

    # Write all variables from meta
    ofile.write('\t\t// properties from meta file\n')
    writeh_vars(ofile, vars)

    # Write fixed functions, constructors, getInstance and init
    str = '\n\tpublic:\n'
    str += '\t\t%s() {};\n' % root
    str += '/** function auto-generated and initialises the class using a passed GetProp instance (handles config file reading).\n* @param gp is a GetProp instance handle.\n* @param qual is a qualifier string [optional] which affects config file interpretation for multiple included property class instances.*/\n' # doxygen annotation
    str += '\t\t%s(GetProp& gp, std::string qual = "") : GetProp(gp) {if (qual.length() > 0) {m_sPrefix = qual + ".";} else {m_sPrefix = "";}}; // constructor\n\n' % root # constructor
    str += '\t\tstatic %s* getInstance();\n\n' % root
    str += '\t\tstatic %s* destroyInstance();\n\n' % root
    str += '/** function auto-generated and reads information from configuration files, calls functions to populate variables and check expected/read variables.\n* @param filename string configuration file.\n* @param argc number of command line arguments over-riding configuration file.\n* @param argv command line arguments over-riding configuration file. */\n' # doxygen annotation
    str += '\t\tvoid init(std::string, int argc = 0, char* argv[] = NULL) throw(Error);\n'
    str += '/** function auto-generated and populates variables (and metalist) from information read from the configuration file. */\n' # doxygen annotation
    str += '\t\tvoid init() throw(Error);\n'
    str += '/** function auto-generated and sets handle of class to provided pointer. @param pHandle pointer to instance of %s class*/\n' % root # doxygen annotation
    str += '\t\tvoid sethandle(%s* pHandle);\n\n' % root
    
    # Write get functions for included properties classes
    if len(cmnclasses) > 0 :
        for cc in cmnclasses :
            cn = len(cmnclasses[cc])
            if cn > 1 or (cn == 1 and cmnclasses[cc][0] != 'my'+cc.lower()) :
                str += '\t\t%s* get%s(std::string qual);\n\n' % (cc, cc)
            else :
                str += '\t\t%s* get%s();\n\n' % (cc, cc)

    # Write vars
    str += '\t\tstd::vector<std::string> getPropListNames();\n\n'
    str += '/** function auto-generated and returns number of configurable parameters duplicated. @return _dups int*/\n' # doxygen annotation
    str += '\t\tint getDups() const;\n\n'
    str += '/** function auto-generated and returns number of configurable parameters missing in configuration file (defaults used). @return _missing int*/\n' # doxygen annotation
    str += '\t\tint getMissing() const;\n\n'
    str += '/** function auto-generated and returns prefix/qualifier for properties class. @return m_sPrefix string*/\n' # doxygen annotation
    str += '\t\tstd::string getPrefix() const;\n\n'
    # unittest()
    str += '/** function auto-generated and runs unittest for member variables */\n' # doxygen annotation
    str += '\t\tstd::string unittest();\n\n'
    ofile.write(str)

    writeh_getfns(ofile, vars)

    # Write main entry point for unit test
    str = "\n"
    str += "\t\t// entry point for self unit test\n"
    str += "\t\tstatic int main(int argc, char* argv[]);\n\n"
    ofile.write(str)

    # Write version string -- note: need to avoid explicit <dollar>Id:....<dollar else SVN will convert it at checkin!
    str = '}; // class %s\n\n' % root
    str += '// rcsid version strings\n'
    str += '#define RCSID_%s \"$' % fname.replace('.','_')
    str += 'Id: %s auto-generated %s from ' % (fname, time.strftime("%Y-%m-%d %H:%M:%S"))
    if len(idtag) > 0 :
        str += '%s' % (idtag.split('Id: ')[1].split('$')[0])
    else :
        str += 'unknown meta tag '
    str += 'using %s $\"\n' % (Version.replace('$','').replace('Id: ',''))
    str += 'extern const std::string RCSID_%scc;\n\n' % fname.replace('.','_')[:-1]
    str += '#endif\n'
    str += "\n// end of file: %s\n" % fname
    ofile.write(str)

    ofile.close()

    return


def writecc(fname, vars, root, cmnclasses, idtag = '') :
    ofile = open(fname, 'w')

    # Write file headers
    str  = genheader(root)
    str += doxheadercc(root)
    str += '#include <iostream>             // std::cout\n'
    str += '#include <sstream>              // std::ostringstream\n'
    str += '#include <algorithm>            // std::find\n'
    str += "\n"
    str += '// logging stuff\n'
    str += '#include <plog/Log.h>\n'
    str += '#include "ExternalMutexConsoleAppender.h"\n'
    str += '#include "SeverityFormatter.h"\n'
    str += "\n"
    str += '// autogenerated header file\n'
    str += '#include \"%s.h\"\n' % root
    str += '\n'
    ofile.write(str)

    # Write tag -- note: these auto-generated codes are not checked in, so we are adding ident of .meta which can be tracked through SVN
    str  = '\nconst std::string RCSID_%s = \"$' % (fname.replace('.','_'))
    str += 'Id: %s auto-generated %s from ' % (fname, time.strftime("%Y-%m-%d %H:%M:%S"))
    if len(idtag) > 0 :
        str += '%s' % (idtag.split('Id: ')[1].split('$')[0])
    else :
        str += 'unknown meta tag '
    str += 'using %s $\";\n\n' % (Version.replace('$','').replace('Id: ',''))
    ofile.write(str)

    # Write constructor/destructor
    str = '%s * %s::handle = NULL;\n\n' % (root, root)
    str += '/** function auto-generated and returns the handle to existing properties class, or if no class instance exists it creates a new instance and returns the handle. Expected usage is when only a single instance of this properties class is needed and therefore this function allows global access.*/\n' # with doxygen annotation
    str += '%s* %s::getInstance(){\n\tif (handle==NULL) {\n\t\thandle = new %s();\n\t}\n\treturn handle;\n} // %s::getInstance\n\n' % (root, root, root, root)
    str += '%s* %s::destroyInstance(){\n\tif (handle!=NULL) {\n\t\tdelete handle;\n\t\thandle = NULL;\n\t}\n\treturn handle;\n} // %s::destroyInstance\n\n' % (root, root, root)

    # getPropList()
    str += '/** function auto-generated and returns complete list of class variables defined in meta file. @return contents of metalist std::vector<std::string>*/\n' # doxygen annotation
    str += 'std::vector<std::string> %s::getPropListNames() {\n' % root
    str += '\tstd::vector<std::string> retList;\n'
    str += '\tfor (std::set<std::string>::iterator it = metalist.begin(); it != metalist.end(); it++) {\n'
    str += '\t\tretList.push_back(*it);\n'
    str += '\t}\n'
    if len(cmnclasses) > 0 :
        for cc in cmnclasses :
            nmq = len(cmnclasses[cc])
            for mq in cmnclasses[cc] :
                str += '\tstd::vector<std::string> %sprops = %s.getPropListNames();\n' % (mq, mq)
                str += '\tfor (std::vector<std::string>::iterator it = %sprops.begin(); it != %sprops.end(); it++) {\n' % (mq, mq)
                str += '\t\tif (std::find(retList.begin(), retList.end(), *it) != retList.end()) {\n'
                str += '\t\t\tdups++;\n\t\t}\n'
                str += '\t\tretList.push_back(*it);\n'
                str += '\t}\n\n' 
    str += '\treturn retList;\n'
    str += '} // %s::getPropListNames\n\n' % root

    # getMissing()
    str += '/** function auto-generated and returns number of configurable parameters missing in configuration file (defaults used). @return _missing int*/\n' # doxygen annotation
    str += 'int %s::getMissing() const {\n\treturn missing;\n}\n\n' % root
    ofile.write(str)

    # getPrefix()
    str = '/** function auto-generated and returns prefix/qualifier for properties class. @return m_sPrefix std::string*/\n' # doxygen annotation
    str += 'std::string %s::getPrefix() const {\n\treturn m_sPrefix;\n}\n\n' % root
    ofile.write(str)



    # Write init functions
    str = 'void %s::init(std::string filename, int argc, char* argv[]) throw(Error) {\n' % root
    str += '\tGetProp::init(filename, argc, argv);\n'
    str += '\tinit();\n'
    str += '\tcheckVars();\n'
    str += '} // %s::init\n\n' % root
    ofile.write(str)

    # Write functions to parse var map
    writecc_varmap(ofile, vars, root, cmnclasses)

    # Write functions to get each var
    writecc_getfns(ofile, vars, root)

    # Write get functions for included properties class instances
    str = ''
    if len(cmnclasses) > 0 :
        for cc in cmnclasses :
            cn = len(cmnclasses[cc])
            if cn > 1 or (cn == 1 and cmnclasses[cc][0] != 'my'+cc.lower()) :
                str += '/** function auto-generated and returns handle to included %s class. @param qual string variable identifying the required instance version of properties class. @return pointer to %s class. Qualifier is defined in the meta, and is then used for the class instance name. */\n' % (cc, cc) # doxygen annotation
                str += '%s* %s::get%s(std::string qual) {\n' % (cc, root, cc)
                for mq in cmnclasses[cc] :
                    str += '\tif (qual == \"%s\") {return &%s;};\n' % (mq, mq)
                str += '\treturn NULL;\n} // %s::get%s\n\n' % (root, cc)
            else :
                str += '/** function auto-generated and returns handle to included %s class. @return pointer to %s class */\n' % (cc, cc) # doxygen annotation
                str += '%s* %s::get%s() {\n' % (cc, root, cc)
                str += '\treturn &%s;\n} // %s::get%s\n\n' % (cmnclasses[cc][0], root, cc)
    ofile.write(str)

    # Write unit test
    writecc_unittestfn(ofile, vars, root, cmnclasses)
    writecc_unittest(ofile, vars, root, cmnclasses)

    # Write tag -- note: need to avoid explicit <dollar>Id:....<dollar else SVN will convert it at checkin!
    #str  = '\nconst std::string RCSID_%s = \"$' % fname.replace('.','_')
    #str += 'Id: %s auto-generated %s by %s $\";\n' % (fname, time.strftime("%Y-%m-%d %H:%M:%S"), Version.replace('$','').replace('Id: ',''))
    #ofile.write(str)

    str = ""
    str += "\n// end of file: %s\n" % fname
    ofile.write(str)

    ofile.close()

    return


def writeconfig(vars) :

    gps = set([vars[h][-1] for h in vars]) 
    for g in gps :
        if g == 'parent' :
            cfile = 'EXAMPLE_%s.cfg' % program
        else :
            cfile = 'EXAMPLE_%s_%s.cfg' % (program, g)

        fout = open(cfile, 'w')
        for v in sorted([i for i in vars if vars[i][-1] == g]) :
            mystr = '{0:<30}{1:<30}# {2:}\n'.format(v, vars[v][1], vars[v][5])
            fout.write(mystr)
        if g == 'parent' :
            fout.write('\n')
            for inc in [i for i in gps if i != 'parent'] :
                fout.write('INCLUDE %s_%s.cfg\n' % (program, inc)) 
        fout.close()

    return


if __name__ == "__main__" :
# below is the generation file for the onsiteProperties.cc file
# edit the meta table file and run the script when adding new parameters
# Meta file has structure:
# - variable name in the config file 
# - variable type (string, int, double etc.)
# - the default value or 'REQUIRED' for a mandatory user-specified value
# - Can also add SENSITIVE to REQUIRED to obfuscate passwords.
# - comment explaining the variable

    Script = sys.argv[0]
    Version = ' '.join(Version.split()[1:6])

    if len(sys.argv) < 2 :
        program = 'onsite'
    else :
        program = sys.argv[1]

    root = '%sProperties' % program
    infile = ''

    # look for meta file in current or optional directories
    paths = [''] # current dir is always checked first
    if len(sys.argv) > 2 :
        paths = paths + sys.argv[2:]

    for path in paths :
        if len(path) > 0 :
            infile = os.path.join(path, '%s.meta' % root)
        else :
            infile = '%s.meta' % root

        if os.path.isfile(infile) :
            break

    if not os.path.isfile(infile) :
        print 'Could not find %s.meta in any directories: %s' % (root, ' '.join(paths)) 
        sys.exit(1)

    print 'running properties generator program for %s using %s\n' % (program, infile)

# read variables table from meta file
    vars, cmnclasses, idtag = readmeta(infile, {})

# write .cc and .h files
    writecc('%s.cc' % root, vars, root, cmnclasses, idtag)
    writeh('%s.h' % root, vars, root, cmnclasses, idtag)

# write example config files
    writeconfig(vars)
