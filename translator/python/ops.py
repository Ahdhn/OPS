#!/usr/bin/env python

"""
OPS source code transformation tool

This tool parses the user's original source code to produce
target-specific code to execute the user's kernel functions.

This prototype is written in Python

usage: ./ops 'file1','file2',...

This takes as input

file1.cpp, file2.cpp, ...

and produces as output modified versions

file1_ops.cpp, file2_ops.cpp, ...

then calls a number of target-specific code generators
to produce individual kernel files of the form

xxx_seq_kernel.cpp -- for single threaded x86 execution
xxx_omp_kernel.cpp -- for OpenMP x86 execution
xxx_kernel.cu -- for CUDA execution

"""

import sys
import re
import datetime

# import OpenMP and CUDA code generation functions
#from op2_gen_seq import op2_gen_seq
#from op2_gen_openmp import op2_gen_openmp
#from op2_gen_cuda import op2_gen_cuda


# from http://stackoverflow.com/a/241506/396967
def comment_remover(text):
    """Remove comments from text"""

    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return ""
        else:
            return s
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    return re.sub(pattern, replacer, text)


def ops_parse_calls(text):
    """Parsing for ops_init/ops_exit"""

    # remove comments just for this call
    text = comment_remover(text)

    inits = len(re.findall('ops_init', text))
    exits = len(re.findall('ops_exit', text))

    return (inits, exits)

def arg_parse(text, j):
    """Parsing arguments in op_par_loop to find the correct closing brace"""

    depth = 0
    loc2 = j
    while 1:
        if text[loc2] == '(':
            depth = depth + 1

        elif text[loc2] == ')':
            depth = depth - 1
            if depth == 0:
                return loc2
        loc2 = loc2 + 1

def get_arg_dat(arg_string, j):
    loc = arg_parse(arg_string, j + 1)
    dat_args_string = arg_string[arg_string.find('(', j) + 1:loc]

    # remove comments
    dat_args_string = comment_remover(dat_args_string)

    # check for syntax errors
    if len(dat_args_string.split(',')) != 3:
        print 'Error parsing op_arg_dat(%s): must have three arguments' \
              % dat_args_string
        return

    # split the dat_args_string into  6 and create a struct with the elements
    # and type as op_arg_dat
    temp_dat = {'type': 'ops_arg_dat',
                'dat': dat_args_string.split(',')[0].strip(),
                'sten': dat_args_string.split(',')[1].strip(),
                'acc': dat_args_string.split(',')[2].strip()}

    return temp_dat

def get_arg_gbl(arg_string, k):
    loc = arg_parse(arg_string, k + 1)
    gbl_args_string = arg_string[arg_string.find('(', k) + 1:loc]

    # remove comments
    gbl_args_string = comment_remover(gbl_args_string)

    # check for syntax errors
    if len(gbl_args_string.split(',')) != 3:
        print 'Error parsing op_arg_gbl(%s): must have four arguments' \
              % gbl_args_string
        return

    # split the gbl_args_string into  4 and create a struct with the elements
    # and type as op_arg_gbl
    temp_gbl = {'type': 'ops_arg_gbl',
                'data': gbl_args_string.split(',')[0].strip(),
                'dim': gbl_args_string.split(',')[1].strip(),
                'acc': gbl_args_string.split(',')[2].strip()}

    return temp_gbl

def ops_par_loop_parse(text):
  """Parsing for op_par_loop calls"""

  loop_args = []

  text = comment_remover(text)
  search = "ops_par_loop_opt"
  i = text.find(search)
  while i > -1:
      arg_string = text[text.find('(', i) + 1:text.find(';', i + 16)]

      # parse arguments in par loop
      temp_args = []
      num_args = 0

      # parse each op_arg_dat
      search2 = "ops_arg_dat"
      search3 = "ops_arg_gbl"
      j = arg_string.find(search2)
      k = arg_string.find(search3)

      while j > -1 or k > -1:
        if k <= -1:
            temp_dat = get_arg_dat(arg_string, j)
            # append this struct to a temporary list/array
            temp_args.append(temp_dat)
            num_args = num_args + 1
            j = arg_string.find(search2, j + 11)

        elif j <= -1:
            temp_gbl = get_arg_gbl(arg_string, k)
            # append this struct to a temporary list/array
            temp_args.append(temp_gbl)
            num_args = num_args + 1
            k = arg_string.find(search3, k + 11)

        elif j < k:
            temp_dat = get_arg_dat(arg_string, j)
            # append this struct to a temporary list/array
            temp_args.append(temp_dat)
            num_args = num_args + 1
            j = arg_string.find(search2, j + 11)

        else:
            temp_gbl = get_arg_gbl(arg_string, k)
            # append this struct to a temporary list/array
            temp_args.append(temp_gbl)
            num_args = num_args + 1
            k = arg_string.find(search3, k + 11)

      temp = {'loc': i,
            'name1': arg_string.split(',')[0].strip(),
            'name2': arg_string.split(',')[1].strip(),
            'dim': arg_string.split(',')[2].strip(),
            'range': arg_string.split(',')[3].strip(),
            'args': temp_args,
            'nargs': num_args}
      #print temp
      loop_args.append(temp)

      i = text.find(search, i + 16)
  print '\n\n'
  return (loop_args)

def main():

  # declare constants

  ninit = 0
  nexit = 0
  nkernels = 0
  consts = []
  kernels = []
  kernels_in_files = []

  OP_GBL = 2

  OP_READ = 1
  OP_WRITE = 2
  OP_RW = 3
  OP_INC = 4
  OP_MAX = 5
  OP_MIN = 6

  OPS_accs_labels = ['OP_READ', 'OP_WRITE', 'OP_RW', 'OP_INC',
                    'OP_MAX', 'OP_MIN']

  #
  # loop over all input source files
  #

  kernels_in_files = [[] for _ in range(len(sys.argv) - 1)]
  for a in range(1, len(sys.argv)):
      print 'processing file ' + str(a) + ' of ' + str(len(sys.argv) - 1) + \
            ' ' + str(sys.argv[a])

      src_file = str(sys.argv[a])
      f = open(src_file, 'r')
      text = f.read()

      #
      # check for ops_init, ops_exit calls
      #

      inits, exits = ops_parse_calls(text)

      if inits + exits > 0:
        print ' '
      if inits > 0:
        print'contains ops_init call'
      if exits > 0:
        print'contains ops_exit call'

      ninit = ninit + inits
      nexit = nexit + exits

      #
      # parse and process op_par_loop calls
      #

      loop_args = ops_par_loop_parse(text)

      for i in range(0, len(loop_args)):
        name = loop_args[i]['name1']
        nargs = loop_args[i]['nargs']
        dim   = loop_args[i]['dim']
        _range   = loop_args[i]['range']
        print '\nprocessing kernel ' + name + ' with ' + str(nargs) + ' arguments'
        print 'dim: '+dim
        print 'range: '+str(_range)

        #
        # process arguments
        #

        var = [''] * nargs
        stens = [0] * nargs
        accs = [0] * nargs
        dims = [''] * nargs #only for globals

        for m in range(0, nargs):
          arg_type = loop_args[i]['args'][m]['type']
          args = loop_args[i]['args'][m]

          if arg_type.strip() == 'ops_arg_dat':
            var[m] = args['dat']
            stens[m] = args['sten']

            l = -1
            for l in range(0, len(OPS_accs_labels)):
                if args['acc'].strip() == OPS_accs_labels[l].strip():
                  break

            if l == -1:
                print 'unknown access type for argument ' + str(m)
            else:
                accs[m] = l + 1

            print var[m]+' '+ str(stens[m]) +' '+str(accs[m])


          if arg_type.strip() == 'ops_arg_gbl':
            var[m] = args['data']
            dims[m] = args['dim']

            l = -1
            for l in range(0, len(OPS_accs_labels)):
                if args['acc'].strip() == OPS_accs_labels[l].strip():
                    break
            if l == -1:
                print 'unknown access type for argument ' + str(m)
            else:
                accs[m] = l + 1

            print var[m]+' '+ str(dims[m]) +' '+str(accs[m])


        #
        # check for repeats
        #
        repeat = False
        rep1 = False
        rep2 = False
        ##
        ##todo
        ##


        #
        # output various diagnostics
        #

        ##
        ##todo
        ##

        #
        # store away in master list
        #

        if not repeat:
            nkernels = nkernels + 1
            temp = {'name': name,
                    'nargs': nargs,
                    'dims': dim,
                    'stens': stens,
                    'var': var,
                    'accs': accs,
                    'range': _range
            }
            kernels.append(temp)
            (kernels_in_files[a - 1]).append(nkernels - 1)
        else:
            append = 1
            for in_file in range(0, len(kernels_in_files[a - 1])):
                if kernels_in_files[a - 1][in_file] == which_file:
                    append = 0
            if append == 1:
                (kernels_in_files[a - 1]).append(which_file)


        #
        # output new source file
        #

        fid = open(src_file.split('.')[0] + '_ops.cpp', 'w')
        date = datetime.datetime.now()
        fid.write('//\n// auto-generated by ops.py on ' +
                  date.strftime("%Y-%m-%d %H:%M") + '\n//\n\n')

        loc_old = 0

        ##
        ##todo
        ##


        fid.close()
        f.close()


        #
        # errors and warnings
        #

        if ninit == 0:
            print' '
            print'-----------------------------'
            print'  ERROR: no call to ops_init  '
            print'-----------------------------'

        if nexit == 0:
            print' '
            print'-------------------------------'
            print'  WARNING: no call to ops_exit  '
            print'-------------------------------'


        #
        # finally, generate target-specific kernel files
        #

        #ops_gen_seq(str(sys.argv[1]), date, consts, kernels)
        #ops_gen_openmp(str(sys.argv[1]), date, consts, kernels)
        #ops_gen_cuda(str(sys.argv[1]), date, consts, kernels)



if __name__ == '__main__':
    if len(sys.argv) > 1:
        main()
    # Print usage message if no arguments given
    else:
        print __doc__
        sys.exit(1)
