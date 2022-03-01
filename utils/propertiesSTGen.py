#!/usr/bin/python
import os
import sys
import time

Version = '$Id: propertiesSTGen.py $'

def readmeta(infile) :
    vars = {}
    cmnclasses = {}
    group = 'parent'
    fin = open(infile,'r')
    idtag = ''
    for l in fin :
        min = ''
        max = ''
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
        if l.startswith('@inherit') :
            #for cc in l.split()[1:] :
            meta = l.split()[1] 
            metaclass = meta.split(os.sep)[-1].replace('.meta','')
            metaqual = l.split()[2:]
            if len(metaqual) > 0 :
                cmnvars, cmncmnclasses, dum = readmeta(meta)
                cmnclasses[metaclass] = {} # creates a list of vars dictionaries
                for mq in metaqual :
                    for cc in cmnvars :
                        newvars = []
                        newvars.append('_%s%s' % (mq, cmnvars[cc][0]))
                        for nv in cmnvars[cc][1:] :
                            newvars.append(nv)
                        cmnclasses[metaclass]['%s.%s'%(mq,cc)] = newvars
            else :
                cmnclasses[metaclass], cmncmnclasses = readmeta(meta)    
            continue
        fs = l.split()
        if fs[1] == 'string' :
            typ = 's' 
        elif fs[1] == 'int' :
            typ = 'i' 
        elif fs[1] == 'float' :
            typ = 'f'
        elif fs[1] == 'double' :
            typ = 'd'
        elif fs[1] == 'bool' :
            typ = 'b'
        else :
            print 'ERROR: invalid type specification for', fs[0]
            return
        if typ in ['i','f','d'] and len(l[:l.find('#')].split()) > 4 :
            min = fs[3]
            max = fs[4]
        if l.find('#') > -1 :
            comment = l[l.find('#'):].replace('#','').rstrip().lstrip()
        else :
            comment = ''
        vars[fs[0]] = ['_%s' % fs[0].lower().replace('.','_'), fs[2], typ, min, max, comment, group]
    fin.close()

    if len([vars[v][0] for v in vars]) > len(set([vars[v][0] for v in vars])) :
        print 'ERROR: duplicate variable name'
        return {}

    return vars, cmnclasses, idtag


def writecc_varmap(ofile, vars, root, cmnclasses, bMain = 0) :
    mandatory = [v for v in vars if len([x for x in vars[v] if x.find("REQUIRED") > -1]) > 0]
    sensitive = [v for v in vars if len([x for x in vars[v] if x.find("SENSITIVE") > -1]) > 0]
 
    str  = '\tint missing = 0;\n'
    str += '\tstd::ostringstream ostrm;\n\n'
    str += '\tostrm << \"%s Config file: \" << config_file << endl;\n\n' % root
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
            str = '\tif ( !contains(\"%s\") ) {\n' % param
            if param in mandatory :
                str += '\t\tthrow Error(config_file + \": Parameter %s not found.\");\n\t}' % (param)
            else :
                if typ == 's' :
                    str += '\t\t%s = \"%s\";' % (var,default.replace('"','').replace("'",""))
                else :
                    str += '\t\t%s = %s;' % (var,default)
                str += '\n\t\tostrm << \"  %s: %s using default value%s\" << endl;' % (param, defaultstr, comment.replace(' and',', field is'))
                str += '\n\t\tmissing++;\n'
                str += '\t}' 
            str += '\n\telse {\n\t\ttry {\n\t\t\t%s = ' % var
            if typ == 's' :
                str += 'getString(\"%s\");' % param
            elif typ == 'i' :
                str += 'getInt(\"%s\");' % param
            elif typ == 'f' :
                str += '(float)getDouble(\"%s\");' % param
            elif typ == 'd' :
                str += 'getDouble(\"%s\");' % param
            elif typ == 'b' :
                str += 'getBool(\"%s\");' % param
            else :
                print 'ERROR: invalid type specification for', param
                return
            if param in sensitive :
                if typ == 's' :
                    str += '\n\t\t\tstring %s = %s.substr(0,1) + "..." + %s.substr(%s.length()-1,1);' % (varstr, var, var, var)
                else :
                    str += '\n\t\t\tstring %s = "...";' % (varstr)
            if param in mandatory :
                str += '\n\t\t\tostrm << \"  %s: \" << %s << \" (field is mandatory%s)\" << endl;' % (param, varstr, comment)
            else :
                str += '\n\t\t\tostrm << \"  %s: \" << %s << \" (default is %s)\" << endl;' % (param, varstr, defaultstr.replace("+\"/\"+"," + ") + comment.replace(' and',', field is'))
            str += '\n\t\t} catch(Error& e) {\n\t\t\tthrow Error(config_file + ": Parameter %s has wrong format!\");\n\t\t}\n' % (param)
            if min > '' and max > '' :
                str += '\t\tif (%s < %s || %s > %s)\n' % (var, min, var, max)
                str += '\t\t\tthrow Error(config_file + ": Parameter %s (" + getString("%s") + ") is not between %s and %s");\n' % (param, param, min, max)
            str += '\t}\n\n'
            ofile.write(str)

    if len(cmnclasses) > 0 :
        str = ''
        for cc in cmnclasses :
            str += '\t%s::init(filename, argc, argv);\n' % cc
        str += '\n'
        ofile.write(str)


    str = ''
    for param in vars :
        str += '\tmetalist.insert(\"%s\");\n' % param
    str += '\n'

    if len(cmnclasses) > 0 :
        for cc in cmnclasses :
            str += '\tstd::vector<std::string>%sprops = %s::getPropListNames();\n' % (cc, cc)
            str += '\tfor (std::vector<std::string>::iterator it = %sprops.begin(); it != %sprops.end(); it++) {\n\t\tmetalist.insert(*it);\n\t}\n\n' % (cc, cc)

    if bMain :
        str += '\tstd::vector<std::string> configList = getConfigListNames(); // Get all properties from config\n'
        str += '\tint unused = 0;\n'
        str += '\tfor (std::vector<std::string>::iterator it = configList.begin(); it != configList.end(); it++) {\n'
        str += '\t\tif (std::find(metalist.begin(), metalist.end(), *it) == metalist.end()) {\n'
        str += '\t\t\tostrm << \"WARNING: config variable \" << *it << \" (" << kvm[*it] << \") is unused!\" << endl;\n'
        str += '\t\t\tunused++;\n'
        str += '\t\t}\n'
        str += '\t}\n'
        str += '\tostrm << "Expected " << metalist.size() << ", read " << configList.size() << ", default " << missing << ", unused " << unused << endl;\n'
        ofile.write(str)
        str = '\tcout << ostrm.str() << endl;\n'

    str += '}\n\n'
    ofile.write(str)

    return 


def writecc_getfns(ofile, vars, root) :
    for param in sorted([v for v in vars if v.find('.') == -1]) :
        var = vars[param][0]
        typ = vars[param][2]
        if typ == 's' :
            str = 'std::string ' 
        elif typ == 'i' :
            str = 'int ' 
        elif typ == 'f' :
            str = 'float '
        elif typ == 'd' :
            str = 'double '
        elif typ == 'b' :
            str = 'bool '
        else :
            print 'ERROR: invalid type specification for', param
            return
        str += '%s::get%s() const {\n\treturn %s;\n}\n\n' % (root, param, var)
        ofile.write(str)

    for param in sorted(set([v.split('.')[1] for v in vars if v.find('.') > -1])) :
        mapparam = sorted([v for v in vars if v.find('.') > -1 and v.split('.')[1] == param])
        typ = vars[mapparam[0]][2]
        if typ == 's' :
            str = 'std::string ' 
        elif typ == 'i' :
            str = 'int ' 
        elif typ == 'f' :
            str = 'float '
        elif typ == 'd' :
            str = 'double '
        elif typ == 'b' :
            str = 'bool '
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


def writecc_unittest(ofile, vars, root, cmnclasses) :
    
    str = '\n'
    str += '#ifdef UNIT_TEST    // unit test of %s\n\n' % root
    str += 'int main(int argc, char* argv[]) {\n'
    str += '\n'
    str += '\tcout << endl << "Begin unit test of %s" << endl;\n' % root
    str += '\n'
    str += '\t%s* prop = %s::getInstance();\n' % (root, root)
    str += '\tif (argc < 2) {\n'
    str += '\t\tcout << "Usage: " << argv[0] << " <config file>" << endl;\n'
    str += '\n'
    str += '\t\tcout << "No file specified, running self contained tests" << endl;\n'
    str += '\n'
    str += '\t\t%s::Self_contained_unittests();\n' % root
    str += '\t\texit(0);\n'
    str += '\t}\n'
    str += '\n'
    str += '\ttry {\n'
    str += '\t\tstd::string filename = argv[1];\n'
    str += '\t\tcout << endl << "Attempting to open file " << filename << endl;\n'
    str += '\t\tprop->init(filename, argc, argv);\n'
    str += '\n'
    str += '\t\tcout << endl << "Trying GetProp file unit tests..." << endl;\n'
    str += '\t\tGetProp::File_unittests(prop);\n'
    str += '\n'
    str += '\t\t} catch (Error& e) {\n'
    str += '\t\t\tcout << "Error using GetProp file unit tests" << endl;\n'
    str += '\t\t\te.print();\n'
    str += '\t\t\texit(1);\n'
    str += '\t\t}\n'
    str += '\n'
    str += '\tcout << endl << "Trying %s unit tests..." << endl;\n' % root
    str += '\ttry {\n'
    str += '\t\tcout << endl << "Exercise generated get methods" << endl;\n'
    for param in [v for v in vars if v.find('.') == -1] :
        str += '\t\tcout << "  get%s() => " << prop->get%s() << endl;\n' % (param, param)
    for param in [v for v in vars if v.find('.') > -1] :
        str += '\t\tcout << "  get%s(%s) => " << prop->get%s(\"%s\") << endl;\n' % (param.split('.')[1], param.split('.')[0], param.split('.')[1], param.split('.')[0])
    for cc in cmnclasses :
        for param in [v for v in cmnclasses[cc] if v.find('.') == -1] :
            str += '\t\tcout << "  get%s() => " << prop->get%s() << endl;\n' % (param, param)
        for param in [v for v in cmnclasses[cc] if v.find('.') > -1] :
            str += '\t\tcout << "  get%s(%s) => " << prop->get%s(\"%s\") << endl;\n' % (param.split('.')[1], param.split('.')[0], param.split('.')[1], param.split('.')[0])
    str += '\t} catch (Error& e) {\n'
    str += '\t\tcout << "Error using generated methods" << endl;\n'
    str += '\t\te.print();\n'
    str += '\t\texit(1);\n'
    str += '\t}\n'
    str += '\n'
    str += '\tcout << endl << "End unit test of %s" << endl;\n' % root
    str += '\n'
    str += '\texit(0);\n'
    str += '}\n'
    str += '#endif // UNIT_TEST of %s\n' % root

    ofile.write(str)

    return 


def writeh_vars(ofile, vars) :
    for param in sorted(vars) :
        var = vars[param][0]
        typ = vars[param][2]
        comment = vars[param][5]
        if typ == 's' :
            type = 'std::string'
        elif typ == 'i' :
            type = 'int'
        elif typ == 'f' :
            type = 'float'
        elif typ == 'd' :
            type = 'double'
        elif typ == 'b' :
            type = 'bool'
        else :
            print 'ERROR: invalid type specification for', param
            return
        str = '\t\t%s %s // %s' % ('{0: <6}'.format(type), '{0: <30}'.format(var +";"), comment)
        str += '\n'

        ofile.write(str)

    return


def writeh_getfns(ofile, vars) :
    seen = []
    for param in vars :
        if param.find('.') > -1 and param.split('.')[1] in seen :
            continue
        typ = vars[param][2]
        if typ == 's' :
            str = '\t\tstd::string ' 
        elif typ == 'i' :
            str = '\t\tint ' 
        elif typ == 'f' :
            str = '\t\tfloat '
        elif typ == 'd' :
            str = '\t\tdouble '
        elif typ == 'b' :
            str = '\t\tbool '
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


def genheader(root) :
    str =  '/*\n'
    str += ' * THIS IS AN AUTOGENERATED FILE.  DO NOT MODIFY BY HAND!\n'
    str += ' * Created by %s\n' % Version
    str += ' * Generated %s\n' % time.strftime("%Y-%m-%d %H:%M:%S")
    str += ' * Edit %s.meta and rerun as follows:\n' % root
    str += ' *     %s %s\n' % (sys.argv[0], sys.argv[1])
    str += ' */\n\n'
    return str

def writeh(fname, vars, root, cmnclasslist, idtag = '') :
    ofile = open(fname, 'w')

    # Write file headers
    str = genheader(root)
    str += '#ifndef __%s_h\n' % root
    str += '#define __%s_h\n' % root
    str += "\n"
    str += '#include <string>           // std::string\n'
    str += '#include <set>              // std::set\n'
    if len(cmnclasslist) == 0 :
	    str += '#include \"GetProp.h\"        // GetProp\n'
    else :
        for cc in cmnclasslist :
            str += '#include \"%s.h\"\n' % cc
    str += '\n'
    ofile.write(str)

    # Write standard 
    if len(cmnclasslist) == 0 :
        str = 'class %s : public GetProp {\n' % root
    else :
        str = 'class %s : public %s {\n' % (root, ' '.join(cmnclasslist.keys()))
    str += '\n\tprotected:\n'
    str += '\t\tstatic %s *handle;\n' % root
    if len(cmnclasslist) == 0 :
        str += '\t\t%s() {};\n' % root    
    else :
        str += '\t\t%s():%s() {};\n' % (root, '(), '.join(cmnclasslist.keys()))
    str += '\t\tstd::set<std::string> metalist;\n'
    str += '\n'
    str += '\t\t// properties from meta file\n'
    ofile.write(str)

    writeh_vars(ofile, vars)

    # Write fixed functions
    str = '\n\tpublic:\n'
    str += '\t\tstatic %s* getInstance();\n' % root
    str += '\t\tstd::vector<std::string> getPropListNames();\n'
    str += '\t\tvoid init(std::string, int argc, char* argv[]) throw(Error);\n'
    str += '\t\tvoid init(std::string) throw(Error);\n'
    str += '\n'
    ofile.write(str)

    writeh_getfns(ofile, vars)

    # Write version string -- note: need to avoid explicit <dollar>Id:....<dollar else SVN will convert it at checkin!
    str = '};\n\n'
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
    ofile.write(str)

    ofile.close()

    return


def writecc(fname, vars, root, cmnclasses, bMain = 0, idtag = '') :
    ofile = open(fname, 'w')

    # Write file headers
    str = genheader(root)
    str += '#include \"%s.h\"\n' % root
    str += '\n'
    str += '#include <iostream>             // std::cout\n'
    str += '#include <sstream>              // std::ostringstream\n'
    str += '#include <algorithm>            // std::find\n'
    str += "\n"
    str += "using namespace std;\n"
    str += "\n"
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
    str = '%s * %s::handle = NULL;\n' % (root, root)
    #str += '%s::%s(){};\n' % (root, root)
    str += '%s* %s::getInstance(){\n\tif (handle==NULL) {\n\t\thandle = new %s();\n\t}\n\treturn handle;\n}\n' % (root, root, root)
    str += '\n'
    str += 'std::vector<std::string> %s::getPropListNames() {\n' % root
    str += '\tstd::vector<std::string> retList;\n'
    str += '\tfor (std::set<std::string>::iterator it = metalist.begin(); it != metalist.end(); it++) {\n'
    str += '\t\tretList.push_back(*it);\n'
    str += '\t}\n'
    str += '\treturn retList;\n'
    str += '}\n'
    str += '\n'
    ofile.write(str)

    # Write init functions
    str  = 'void %s::init(std::string filename) throw(Error) {\n' % root
    str += '\tinit(filename, 0, NULL);\n'
    str += '}\n'
    str += '\n'
    str += 'void %s::init(std::string filename, int argc, char* argv[]) throw(Error) {\n' % root
    if bMain :
        str += '\tGetProp::init(filename, argc, argv);\n'
    ofile.write(str)

    # Write functions to parse var map
    writecc_varmap(ofile, vars, root, cmnclasses, bMain)

    # Write functions to get each var
    writecc_getfns(ofile, vars, root)

    # Write unit test
    if bMain :
        writecc_unittest(ofile, vars, root, cmnclasses)

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
# below is the generation file for the onsitePropertiesST.cc file
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

    print 'running propertiesST generator program for %s' % program

    root = '%sPropertiesST' % program
    infile = '%s.meta' % root

# read variables table from meta file
    vars, cmnclasses, idtag = readmeta(infile)

# write .cc and .h files
    writecc('%s.cc' % root, vars, root, cmnclasses, 1, idtag)
    writeh('%s.h' % root, vars, root, cmnclasses, idtag)
    for cc in cmnclasses : 
        writecc('%s.cc' % cc, cmnclasses[cc], cc, {}, 0)
        writeh('%s.h' % cc, cmnclasses[cc], cc, {})

# write example config files
    writeconfig(vars)
