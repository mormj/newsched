pmt_gen_h = custom_target('flatc-output',
                        build_always_stale : true,
                        input : ['pmt.fbs'],
                        output : ['pmt_generated.h'],
                        command : [flatc, '-c', '--reflect-types','--gen-mutable','--reflect-names','--cpp-std','c++17', '-o', '@OUTDIR@/../include/pmt', '@INPUT@'])

pmt_gen_h_dep = declare_dependency(sources : [pmt_gen_h])