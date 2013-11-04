"""
OPS Sequential code generator

This routine is called by ops.py which parses the input files

It produces a file xxx_seq_kernel.cpp for each kernel,
plus a master kernel file

"""

import re
import datetime

def comm(line):
  global file_text, FORTRAN, CPP
  global depth
  prefix = ' '*depth
  if len(line) == 0:
    file_text +='\n'
  else:
    file_text +=prefix+'//'+line+'\n'

def code(text):
  global file_text, g_m
  global depth
  prefix = ''
  if len(text) != 0:
    prefix = ' '*depth
  #file_text += prefix+rep(text,g_m)+'\n'
  file_text += prefix+text+'\n'

def FOR(i,start,finish):
  global file_text
  global depth
  code('for ( int '+i+'='+start+'; '+i+'<'+finish+'; '+i+'++ ){')
  depth += 2

def WHILE(line):
  global file_text
  global depth
  code('while ( '+ line+ ' ){')
  depth += 2

def ENDWHILE():
  global file_text
  global depth
  depth -= 2
  code('}')

def ENDFOR():
  global file_text
  global depth
  depth -= 2
  code('}')

def IF(line):
  global file_text
  global depth
  code('if ('+ line + ') {')
  depth += 2

def ELSEIF(line):
  global file_text
  global depth
  code('else if ('+ line + ') {')
  depth += 2

def ELSE(line):
  global file_text
  global depth
  code('else {')
  depth += 2

def ENDIF():
  global file_text
  global depth
  depth -= 2
  code('}')

def ops_gen_seq_macro(master, date, kernels):

  global dims, stens
  global g_m, file_text, depth

  OP_ID   = 1;  OP_GBL   = 2;  OP_MAP = 3;

  OP_READ = 1;  OP_WRITE = 2;  OP_RW  = 3;
  OP_INC  = 4;  OP_MAX   = 5;  OP_MIN = 6;

  accsstring = ['OP_READ','OP_WRITE','OP_RW','OP_INC','OP_MAX','OP_MIN' ]


##########################################################################
#  create new kernel file
##########################################################################

  for nk in range (0,len(kernels)):
    arg_typ  = kernels[nk]['arg_type']
    name  = kernels[nk]['name']
    nargs = kernels[nk]['nargs']
    dim   = kernels[nk]['dim']
    dims  = kernels[nk]['dims']
    stens = kernels[nk]['stens']
    var   = kernels[nk]['var']
    accs  = kernels[nk]['accs']
    typs  = kernels[nk]['typs']


##########################################################################
#  start with seq kernel function
##########################################################################

    g_m = 0;
    file_text = ''
    depth = 0
    n_per_line = 4

    i = name.find('kernel')
    name2 = name[0:i-1]
    #print name2

    #backend functions that should go to the sequential backend lib
    code('#include "lib.h"')

    comm('user function')
    code('#include "'+name2+'_kernel.h"')
    comm('')
    comm(' host stub function')
    code('void ops_par_loop_'+name+'(char const *name, int dim, int* range,')
    text = ''
    for n in range (0, nargs):

      text = text +' ops_arg arg'+str(n)
      if nargs <> 1 and n != nargs-1:
        text = text +','
      else:
        text = text +') {'
      if n%n_per_line == 3 and n <> nargs-1:
         text = text +'\n'
    code(text);
    depth = 2

    code('');
    code('char *p_a['+str(nargs)+'];')
    code('int  offs['+str(nargs)+'][2];')
    code('int  count[dim];\n')

    text ='ops_arg args['+str(nargs)+'] = {'
    for n in range (0, nargs):
      text = text +' arg'+str(n)
      if nargs <> 1 and n != nargs-1:
        text = text +','
      else:
        text = text +'};\n\n'
      if n%n_per_line == 5 and n <> nargs-1:
        text = text +'\n                    '
    code(text);

    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        code('offs['+str(n)+'][0] = 1;  //unit step in x dimension')
        code('offs['+str(n)+'][1] = ops_offs_set(range[0],range[2]+1, args['+str(n)+']) - ops_offs_set(range[1],range[2], args['+str(n)+']);')
        IF('args['+str(n)+'].stencil->stride[0] == 0')
        code('offs['+str(n)+'][0] = 0;')
        code('offs['+str(n)+'][1] = args['+str(n)+'].dat->block_size[0];')
        ENDIF();
        comm('stride in x as y stride is 0')
        ELSEIF('args['+str(n)+'].stencil->stride[1] == 0')
        code('offs['+str(n)+'][0] = 1;')
        code('offs['+str(n)+'][1] = -( range[1] - range[0] );')
        ENDIF()
        code('')


    #comm('stride in y as x stride is 0')
    #IF('args[i].stencil->stride[0] == 0')
    #code('offs[i][0] = 0;')
    #code('offs[i][1] = args[i].dat->block_size[0];')
    #ENDIF();
    #comm('stride in x as y stride is 0')
    #ELSEIF('args[i].stencil->stride[1] == 0')
    #code('offs[i][0] = 1;')
    #code('offs[i][1] = -( range[1] - range[0] );')
    #ENDIF()
    #ENDIF()
    #ENDFOR()

    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        code('int off'+str(n)+'_1 = offs['+str(n)+'][0];')
        code('int off'+str(n)+'_2 = offs['+str(n)+'][1];')
        code('int dat'+str(n)+' = args['+str(n)+'].dat->size;')

    code('')
    comm('set up initial pointers')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        code('p_a['+str(n)+'] = (char *)args['+str(n)+'].data')
        code('+ args['+str(n)+'].dat->size * args['+str(n)+'].dat->block_size[0] * ( range[2] * args['+str(n)+'].stencil->stride[1] - args['+str(n)+'].dat->offset[1] )')
        code('+ args['+str(n)+'].dat->size * ( range[0] * args['+str(n)+'].stencil->stride[0] - args['+str(n)+'].dat->offset[0] );')
        code('')
      else:
        code('p_a['+str(n)+'] = (char *)args['+str(n)+'].data;')
        code('')

    code('')


    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        code('xdim'+str(n)+' = args['+str(n)+'].dat->block_size[0];')
    code('')

    FOR('n_y','range[2]','range[3]')
    FOR('n_x','range[0]','range[1]')

    comm('call kernel function, passing in pointers to data')
    code('')
    text = name+'( '
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
        text = text +' ('+(str(typs[n]).replace('"','')).strip()+' *)p_a['+str(n)+']'
      else:
        text = text +' ('+(str(typs[n]).replace('"','')).strip()+' *)p_a['+str(n)+']'
      if nargs <> 1 and n != nargs-1:
        text = text + ','
      else:
        text = text +' );\n'
      if n%n_per_line == 2 and n <> nargs-1:
        text = text +'\n          '
    code(text);
    code('')


    comm('shift pointers to data x direction')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
          #code('p_a['+str(n)+']= p_a['+str(n)+'] + (args['+str(n)+'].dat->size * offs['+str(n)+'][0]);')
          code('p_a['+str(n)+']= p_a['+str(n)+'] + (dat'+str(n)+' * off'+str(n)+'_1);')

    ENDFOR()
    code('')

    comm('shift pointers to data y direction')
    for n in range (0, nargs):
      if arg_typ[n] == 'ops_arg_dat':
          #code('p_a['+str(n)+']= p_a['+str(n)+'] + (args['+str(n)+'].dat->size * offs['+str(n)+'][1]);')
          code('p_a['+str(n)+']= p_a['+str(n)+'] + (dat'+str(n)+' * off'+str(n)+'_2);')
    ENDFOR()


    depth = depth - 2
    code('}')

##########################################################################
#  output individual kernel file
##########################################################################
    fid = open(name+'_seq_kernel.cpp','w')
    date = datetime.datetime.now()
    fid.write('//\n// auto-generated by ops.py on '+date.strftime("%Y-%m-%d %H:%M")+'\n//\n\n')
    fid.write(file_text)
    fid.close()

# end of main kernel call loop

##########################################################################
#  output one master kernel file
##########################################################################

  file_text =''
  comm('header')
  code('#include "ops_lib_cpp.h"')
  code('')

  comm('user kernel files')

  kernel_name_list = []

  for nk in range(0,len(kernels)):
    if kernels[nk]['name'] not in kernel_name_list :
      code('#include "'+kernels[nk]['name']+'_seq_kernel.cpp"')
      kernel_name_list.append(kernels[nk]['name'])

  master = master.split('.')[0]
  fid = open(master.split('.')[0]+'_seq_kernels.cpp','w')
  fid.write('//\n// auto-generated by op2.py on '+date.strftime("%Y-%m-%d %H:%M")+'\n//\n\n')
  fid.write(file_text)
  fid.close()
