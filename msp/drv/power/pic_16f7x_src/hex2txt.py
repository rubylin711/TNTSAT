def hex2v(fin_name, fout_name):
    flines = file(fin).readlines()
    fout   = file(fout_name,'w')

    max_addr = 0

    from time import localtime, strftime
    print strftime("// %a, %d %b %Y %H:%M:%S ", localtime() )

    for line in flines:
        ndate_word = int(line[1:3],16)/2
        start_addr = int(line[3:7],16)/2

        if max_addr < (start_addr+ndate_word) :
            max_addr  = start_addr+ndate_word
    print max_addr

    rom_dat = [ 0 ] * max_addr

    for line in flines:
        ndate_word = int(line[1:3],16)/2
        start_addr = int(line[3:7],16)/2

        #~ print ndate_word, start_addr
        for i in range(ndate_word):
            t = 9+i*4
            b1 = line[t   : t+2 ]
            b2 = line[t+2 : t+4 ]
            t = int(b2+b1, 16)

            if max_addr < (i+start_addr) :
                max_addr  = i+start_addr

            rom_dat[i+start_addr] = t
            
#    print>> fout,  ""      
    for i,d in enumerate(rom_dat):
# 		if ( i < ( max_addr - 1 ) ):
 		  if ( i < max_addr ):
 			print>> fout,  "0x%04X," %(d)
# 		else:
# 			print>> fout,  "0x%04X};" %(d)
#	print>> fout,"};"
	
if __name__ == '__main__':
    import sys
    fin  =sys.argv[1]
    fout =sys.argv[2]
    hex2v(fin, fout)


