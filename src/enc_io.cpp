#include <strsuite/encmetric/enc_io.hpp>
using namespace sts;

size_t sts::stdin_getChrs(iochar_pt ptr, size_t len){
	if(len == 0)
		return 0;
	size_t siz = 0;
	size_t chrRead = 0;
	bool eof;

    uint unity = ptr.unity();
	do{
		siz = raw_stdin_readbytes(ptr.data(), (len - chrRead) * unity);
		eof = siz < (len - chrRead);//less character than required
		while(siz > 0){
            if(siz < unity){
                if(raw_stdin_readbytes((ptr+siz).data(), unity - siz) < (unity - siz))
                    throw incorrect_encoding{"Incomplete character"};
                siz = unity;
            }
			size_t dwsiz = ptr.chLen();
			if(dwsiz > siz){
				if(raw_stdin_readbytes((ptr+siz).data(), dwsiz - siz) < (dwsiz - siz))
					throw incorrect_encoding{"Incomplete character"};
				siz = dwsiz;
			}
			siz -= dwsiz;
			if(!ptr.valid_next(siz)){
                throw incorrect_encoding{"Invalid character encoding"};
            }
			chrRead++;	
		}
	}
	while(!eof && chrRead != len);
	return chrRead;
}

size_t sts::stdin_getChrs_validate(iochar_pt ptr, size_t len){
	if(len == 0)
		return 0;
	size_t siz = 0;
	size_t chrRead = 0;
	bool eof;

    uint unity = ptr.unity();
	do{
		siz = raw_stdin_readbytes(ptr.data(), (len - chrRead) * unity);
		eof = siz < (len - chrRead);//less character than required
		while(siz > 0){
            if(siz < unity){
                if(raw_stdin_readbytes((ptr+siz).data(), unity - siz) < (unity - siz))
                    throw incorrect_encoding{"Incomplete character"};
                siz = unity;
            }
			size_t dwsiz = ptr.chLen();
			if(dwsiz > siz){
				if(raw_stdin_readbytes((ptr+siz).data(), dwsiz - siz) < (dwsiz - siz))
					throw incorrect_encoding{"Incomplete character"};
				siz = dwsiz;
			}
			siz -= dwsiz;
			ptr.next();
			chrRead++;
		}
	}
	while(!eof && chrRead != len);
	return chrRead;
}

size_t sts::stdout_putChrs(c_iochar_pt ptr, size_t len){
	if(len == 0)
		return 0;
	size_t siz = 0;
	size_t chrWrite = 0;
	bool eof;

    uint unity = ptr.unity();
	do{
		siz = raw_stdout_writebytes(ptr.data(), (len - chrWrite) * unity);
		if(siz < 0)
			throw encoding_error{"IO error"};
		eof = siz < (len - chrWrite);//less character than required
		while(siz > 0){
            if(siz < unity){
                if(raw_stdout_writebytes((ptr+siz).data(), unity - siz) < (unity - siz))
                    throw incorrect_encoding{"Incomplete character"};
                siz = unity;
            }
			size_t dwsiz = ptr.chLen();
			if(dwsiz > siz){
				if(raw_stdout_writebytes((ptr+siz).data(), dwsiz - siz) < (dwsiz - siz))
					throw incorrect_encoding{"Incomplete character"};
				siz = dwsiz;
			}
			siz -= dwsiz;
			ptr.next();
			chrWrite++;
		}
	}
	while(!eof && chrWrite != len);
	return chrWrite;
}

size_t sts::stderr_putChrs(c_iochar_pt ptr, size_t len){
	if(len == 0)
		return 0;
	size_t siz = 0;
	size_t chrWrite = 0;
	bool eof;

    uint unity = ptr.unity();
	do{
		siz = raw_stderr_writebytes(ptr.data(), (len - chrWrite) * unity);
		eof = siz < (len - chrWrite);//less character than required
		while(siz > 0){
            if(siz < unity){
                if(raw_stderr_writebytes((ptr+siz).data(), unity - siz) < (unity - siz))
                    throw incorrect_encoding{"Incomplete character"};
                siz = unity;
            }
			size_t dwsiz = ptr.chLen();
			if(dwsiz > siz){
				if(raw_stderr_writebytes((ptr+siz).data(), dwsiz - siz) < (dwsiz - siz))
					throw incorrect_encoding{"Incomplete character"};
				siz = dwsiz;
			}
			siz -= dwsiz;
			ptr.next();
			chrWrite++;
		}
	}
	while(!eof && chrWrite != len);
	return chrWrite;
}
