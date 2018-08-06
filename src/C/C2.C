/*
 *  Released under the GNU GPL.  See http://www.gnu.org/licenses/gpl.txt
 *
 *  This program is part of the DeSmet C Compiler
 *
 *  DeSmet C is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundatation; either version 2 of the License, or any
 *  later version.
 *
 *  DeSmet C is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 */
/*	 C88 COMPILER			C2.C	*/

#include "PASS1.H"
#include "NODES.H"
#if CHECK
#include "OBJ.H"
#endif

dolf(char skipit) {
	char *memchr(), *mcp;
	int lsiz;

	if (cur > savEnd) {
		if(cur < macEnd)
			macxp=macfrom[nested];
		cur=nestfrom[--nested];
		whitesp();
		}
	else {
		if (addparm == 0 && addproto == 0) cline+=lineinc;
		lineinc=0;
		newline=1;			/* generate STMT node	*/
		last=lineBeg=++cur;
		if(eofptr && cur>=eofptr) return cur=eofptr;
		while(1){
			while(cur=memchr(cur, LF, bufEnd-cur)){
				lineinc++;
				if(*(cur-1) == '\\') {
					memcpy(cur-1, cur+1, (bufEnd-cur)-1);
					bufEnd-=2;
					cur--;
					continue;
					}
				if(*(cur-2) == '\\'){
					memcpy(cur-2, cur+1, (bufEnd-cur)-1);
					bufEnd-=3;
					cur-=2;
					continue;
					}
				lineEnd=cur+1;
				cur=lineBeg;
				if(trigraphs) {
					char *tp;

					while(tp=cur=memchr(cur, '?', lineEnd-cur)){
						if(*++cur == '?'){
							switch(*(cur+1)){
								case '=':	*tp='#';
											break;
								case '(':	*tp='[';
											break;
								case '/':	*tp='\\';
											break;
								case ')':	*tp=']';
											break;
								case '\'':	*tp='^';
											break;
								case '<':	*tp='{';
											break;
								case '!':	*tp='|';
											break;
								case '>':	*tp='}';
											break;
								case '-':	*tp='~';
											break;
								default:	continue;
								}
							memcpy(cur, cur+2, (bufEnd-cur)-2);
							bufEnd-=2;
							lineEnd-=2;
							}
						}
					cur=lineBeg;
					}
				if (skipit == 0) {
					while (ltype[*cur] == SPACE) cur++;
					if (*cur == '#') preproc();
					whitesp();
					}
				return;
				}
			if(addparm)
				mcp = addparm;
			else if(macproto)
				mcp = macproto;
			else if(addproto)
				mcp = addproto;
			else mcp = lineBeg;
			if((lsiz=bufEnd-mcp) > 512) {
				error("line too long");
				real_exit(2);
				}
			else
				memcpy(&fileBuf[-lsiz], mcp, lsiz);
			lineBeg=&fileBuf[lineBeg-bufEnd];
			if(addparm)addparm=&fileBuf[-lsiz];
			else if(addproto)addproto=&fileBuf[-lsiz];
			if (see_exit && incnext == 0) bufEnd = fileBuf + see_call(0,fileBuf,2048);
			else bufEnd = fileBuf + read(file, fileBuf, 2048);
			if(bufEnd < fileBuf)
				error("file read error");
			if(bufEnd < savEnd) {
				*bufEnd++=LF;
				*bufEnd++=LF;
				eofptr=bufEnd;
				*bufEnd++=CONTZ;
				}
			cur = fileBuf;
			}
		}
	}

pswitch(char *sw, int def) {
	char c;

	while(ltype[*cur] == LETTER) cur++;
	while(ltype[*cur] == SPACE) cur++;
	if(*cur == '=') *sw = def;
	else if(*cur == '-') *sw = 0;
	else if(*cur == '!') *sw = !sw;
	else *sw = 1;
	}

	
preproc() {
	char want;
	cur++;
	whitesp();
	if(*cur == LF)
		return;
	tokit();
	if (heir != RESERVED && heir != UNDEF && heir != OPERAND)
		error_l("bad control");

	else switch(string[0]) {
		case 'a':	if (match("asm"))
						if(xkwd) return cur=tokat;		/* backup so statement will find ASM	*/
						else error("#asm option not on");
					break;
		case 'd':	if (match("define"))
						return adddef();
					break;
		case 'e':	if (match("elif") || match("else"))
						return skipsome(0);
					else if (match("endif"))
ppexit:					return skipl();
					else if (match("error")) {
						char *cp;

						cp = string;
						while(cur < lineEnd)
							*cp++ = *cur++;
						while(--cp >= string && *cp <= 0x20)
							;
						*(cp+1) = '\0';
						e_msg(-1,string);
						goto ppexit;
						}
					break;
		case 'i':	if (match("include"))
						return doinc();
					else if (match("if")) {
						long cexpr;
ifproc:					mactokn=1;
						tokit();
						cexpr=constexp();
						mactokn=0;
						if (cexpr == 0)
							if(skipsome(1))
								goto ifproc;
						goto ppexit;
						}
					else if (match("ifdef") || match("ifndef")) {
						want=match("ifdef");
						if (ltype[*cur++] != LETTER)
							error_l("invalid identifier in #ifdef/#ifndef");
						else {
							find(1);
							if ((heir == DEFINED && want) ||
							(heir != DEFINED && want == 0))
								goto ppexit;
							if(skipsome(1))
								goto ifproc;
							goto ppexit;
							}
						}
					break;
		case 'l':	if (match("line")) {
						tokit();
						if (heir != CONSTANT)
							error_l("invalid digit-sequence in #line");
						else {
							cline=wvalue;
							}
						whitesp();
						if(*cur == LF)
							return;
						tokit();
						if (heir != STRNG)
							error_l("invalid string-literal in #line");
						else
							strcpy(name, string);
						goto ppexit;
						}
					break;
		case 'p':	if (match("pragma")) {
						switch(toupper(*cur++)){
							case 'T':	pswitch(&trigraphs, 0);
										break;
							case 'W':	pswitch(&wopt, 0);
										break;
							case 'E':	if(toupper(*cur) == 'X') {
											pswitch(&xkwd, 0);
											__stdc = !xkwd;
											}
										break;
							}
						goto ppexit;
						}
					break;
		case 'u':	if (match("undef")) {
						if (ltype[*cur++] != LETTER)
							error_l("invalid identifier in #undef");
						else {
							find(1);
							if (heir == RESERVED && bvalue >= R_LINE && bvalue <= R_STDC)
								error_l("cannot #undef predefined names");
							else if (heir != DEFINED)
								error_l("#undef identifier not defined");
							else (nameat-2)->byte=' ';
							}
						goto ppexit;
						}
					break;
		}
	error_l("bad control");
	}

skipsome(char elifOK) {
	char ifnest, eltype;
	ifnest=1;
	while (ifnest) {
		skipl();
		dolf(1);
		while (ltype[*cur] == SPACE) cur++;
		if (*cur == CONTZ) return;
		if (*cur == '#' && *(cur+1) != '\r' && *(cur+1) != '\n') {
			eltype=0;
			tokit();
			tokit();
			if (heir == RESERVED || heir == UNDEF || heir == OPERAND) {
				if (match("if") || match("ifdef") || match("ifndef"))
					ifnest++;
				else if (match("endif")) ifnest--;
				else if (elifOK && match("else") && ifnest == 1)
					ifnest=0;
				else if (elifOK && match("elif") && ifnest == 1)
					return 1;
				}
			}
		}
	skipl();
	return 0;
	}

match(mat)
	char *mat; {
	char *str;

	str=string;
	do
		if (*str != *mat++) return 0;
	while (*str++);
	return 1;
	}

adddef() {
	char *newdef,*olddef,*defat,i,lastdef, wsp=0;

	olddef=0;

/*	do not do normal tokit as may be redefining a define	*/
	tokat=cur;
	curch=*cur++;
	if (ltype[curch] == LETTER) {
		find(1);
		whitesp();
		if(strncmp(tokat, "defined", 7) == 0 ||
			(heir == RESERVED && bvalue >= R_LINE && bvalue <= R_STDC)) {
				error("Can't redefine");
				return;
			}
		if (heir == DEFINED) {		/* kill old define	*/
			*(nameat-2)='0';		/* with zero as first letter */
			olddef=nameat;
			search(1);
			}
		}
	else {
		cur--;
		tokit();
		}

	wp=mfree;
	*wp=machash[hashno];
	machash[hashno]=wp;
	j=0;
	nameat=wp+1;
	newdef=string;
	while(*nameat++=*newdef++) j++;
	defat=nameat;

	defat->defcl=DEFINED;
	defat->dnlen=j+1;
	if(blevel) {
		defat->dchain=hash[32];
		hash[32]=defat;
		}
	defat->dargs=255;
	newdef=&defat->dval;
	if (*cur == '(' && *(cur-1) != ' ' && *(cur-1) != '\11') {
		defat->dargs=0;
		cur++;
		whitesp();
		if (*cur == ')') {
		cur++;
			whitesp();
			goto no_args;
			}
		if (maxmem-mfree < 1200) {
			error("out of memory");
			real_exit(2);
			}
		mfree+=1000;
		do {
			whitesp();
			tokat=cur;
			curch=*cur++;
			find(1);
			whitesp();
			if (heir && heir > 4 && heir != OPERAND && heir != UNDEF) {
				error("invalid parameter");
				skipl();
				goto cleanup;
				}
			newname();
			nameat->defcl=DEFPARM;
			nameat->dpnum=++defat->dargs;
			mfree=nameat+2;
			}
		while (*cur++==',');
		if (*(cur-1) != ')') {
			error("need closing parenthesis");
			skipl();
			goto cleanup;
			}
		}

no_args:
	lastdef=0;
	whitesp();
	while (*cur != LF) {
		wsp=0;
		tokat=cur;
		if(*cur==CR) { cur++; continue; }
		if(*cur=='#') {
			if(*++cur=='#') {
				if(!lastdef) {
					error("## can't be first");
					skipl();
					goto cleanup;
					}
				if(lastdef==DEFPARM) {
					*newdef=*(newdef-1);
					*(newdef-1)=DEFCPY;
					newdef++;
					}
				*newdef++=lastdef=DEFPAS;
				cur++;
				whitesp();
				continue;
				}
			*newdef++=lastdef=DEFSFY;
			whitesp();
			continue;
			}
		switch(ltype[curch=*cur++]) {
			case LETTER: find(1);
						if (heir == DEFPARM) {
							*newdef++=nameat->dpnum;
							lastdef=DEFPARM;
							continue;
							}
						else if(heir == DEFINED && nameat == defat) {
							if(lastdef == DEFSTR) newdef--;
							else *newdef++=DEFSTR;
							lastdef=DEFSTR;
							*newdef++='$';
							while (tokat < cur) *newdef++=*tokat++;
							*newdef++=LF;
							continue;
							}
						else {
defstring:					if(lastdef == DEFSFY) {
								error("parameter must follow #");
								newdef--;
								lastdef=0;
								goto cleanup;
								}
							if(lastdef == DEFSTR) newdef--;
							else *newdef++=DEFSTR;
							lastdef=DEFSTR;
							if(wsp && *(newdef-1) != ' ') *newdef++=' ';
							else while (tokat < cur) *newdef++=*tokat++;
							*newdef++=LF;
							continue;
							}
			case SPACE:
defspace:				whitesp();
						if(lastdef==DEFPARM && *cur=='#' && *(cur+1) =='#')
							continue;
						wsp++;
						goto defstring;
			case 5:
			case 8:		while(*cur++ != curch) if(*(cur-1)=='\\') cur++;
						goto defstring;
			default:	if(curch == '/' && *cur == '*') {
							cur--;
							goto defspace;
							}
						while((curch=ltype[*cur]) != LETTER && curch != SPACE
						&& *cur != '"' && *cur != '\'' && *cur != LF && *cur != '/')
									cur++;
						goto defstring;
			}
		
		}
cleanup:
	if(lastdef == DEFPAS || lastdef == DEFSFY) {
		error("#(#) can't be last");
		newdef--;
		}
	*newdef=DEFEND;
	mfree=newdef+1;
	for (i=0; i < 32; i++)
		while (hash[i] > mfree) hash[i]=hash[i]->word;
	}



/*	add_define  --  add a 'n' option from command tail- a define.	*/

add_define() {
	char *defat,*newdef;

	tokit();			/*	name of variable	*/
	newname();
	defat=nameat;
	defat->defcl=DEFINED;
	defat->dargs=255;
	newdef=&defat->dval;
	*newdef++=DEFSTR;
	if (*cur == 0) {		/* if no value, assume 1	*/
		*newdef++='1';
		}
	else if (*cur == '=') while (*++cur) *newdef++=*cur;
	else {
		os("illegal N option ");
		real_exit(2);
		}

	*newdef++=LF;
	*newdef=DEFEND;
	mfree=newdef+1;
	}

xsetblock(){
#asm
	dseg
	public	_pcb_:word
oldsp	rw	1
oldbp	rw	1
	cseg
	mov	oldsp,sp
	mov	oldbp,bp
	mov	ax,_pcb_
	cli
	mov	ss,ax
	mov	sp,100H
	sti
	mov	bx,ds
	add	bx,1000H
	mov	es,ax
	sub	bx,es:[2CH]
	mov	ah,4AH
	int	21H
	mov	ax,ds
	cli
	mov	ss,ax
	mov	sp,oldsp
	sti
	mov	bp,oldbp
#
	}


xalloc(int size){
#asm
	mov	ah,48H
	mov	bx,#size
	add bx,15
	mov	cl,4
	shr	bx,cl
	int	21H
	jnc	ok
	xor	ax,ax
ok:
#
	}

doinc() {
	int  i, ifd;
	char inc_search;
	char inctemp[66];

	if (incnext == MAXINC) { error_l("include nesting too deep"); return;}
/*	accept <> around an include name	*/

	if(*cur!='<'&&*cur!='"') {
		tokit();					/* expand macro */
		if(curch == '"'){
			inc_search=0;
			goto inc_nest;
			}
		if(curch == '<') {
			inc_search=1;
			goto inc_scan;
			}
		goto bad_inc;
		}
	if (*cur == '<') inc_search=1;
	else if (*cur == '"') inc_search=0;
	else
bad_inc: error_l("bad include");

	cur++;
inc_scan:
	i=0;
	while (*cur != '"' && *cur != '>' && i < 200) {
		if (*cur == '\n' || *cur == CONTZ) {
			error_l("unmatched \"");
			return;
			}
		string[i++]=*cur++;
		}
	if (i == 200) { error_l("string too long"); return; }
	cur++;
	string[i]=0;
inc_nest:
	skipl();

/*	supply a drive name of i option was specified	*/

	if (istring[0]) {
		if (string[1] != ':') strcpy(&string[100],string);
		else strcpy(&string[100],&string[2]);
		strcpy(string,istring);		/*	copy in -i prefix	*/
		strcat(string,&string[100]);/*	add name to end */
		}

/*	if MS-DOS V2.0 and use include= to find file */
	extern char _msdos2;
	if (inc_search && _msdos2) {
		findfile(string,inctemp);
		strcpy(string,inctemp);
		}

	if ((ifd=open(string,0)) == -1) {
		os("cannot open ");
		os(string);
		ocrlf();
		real_exit(2);
		}
	if(incPara[incnext]==0 && (incPara[incnext]=xalloc(SAVSIZE))==0)
		error_l("not enough #include buffer space");
	else {
		if(incnext==0) lastline=cline;
		_lmove(SAVSIZE, name, _showds(), 0, incPara[incnext++]);
		eofptr=cline=0;
		file=ifd;
		strcpy(name,string);
		bufEnd=savEnd;
		macname[nested]=0;
		cur=nestfrom[nested]=bufEnd-1;
		lineinc=1;
		*cur=LF;
		dolf(0);
		}
	}

doinit() {
	char isstatic,*varis,*bptr;
	int  value;

	value=0;
	if (addat->nstor == STYPEDEF || original) {
		original=0;
		return 0;
		}
	isstatic=addat->nstor <= SEXTERN;
	if (isstatic) {
		ctlb(1);
		if (is_big) {
			if ((was_ext || addat->nstor == SEXTERN || addat->nstor == SEXTONLY) && 
				(addat->ntype[0] == CSTRUCT || addat->ntype[0] == ARRAY))
				ctlb(addat->nstor + 128);
			else ctlb(addat->nstor);
			}
		else ctlb(addat->nstor);
		ctlw(addat->noff);

/*	make the names of statics within functions distinct	*/
		if (addat->nstor == SSTATIC && funname) {
			bptr=funname;
			while (*bptr) ctlb(*bptr++);
			ctlb('_');
			}
		ctls(addat-addat->nlen);
		}
	varis=&addat->ntype[0];
	locplus=0;	/* local offset from locoff */
	if (ifch('=')) {
		if (*varis == CSTRUCT && curch != '{') {
			error("need '{' for STRUCT initilization");
			return 0;
			}
		if (addat->nstor == SEXTONLY) {
			error("cannot initilize EXTERN");
			return 0;
			}
		value=initsome(varis,1,isstatic,0);
		if (isstatic) ctlb(INITEND);
		}
	else {
		if (isstatic && addat->nstor != SEXTONLY) {
			if (addat->ntype[0] != FUNCTION) {
				ctlb(INITRB);
				ctlw(dsize(varis));
				}
			else ctlb(INITFUN);
			}
		}
	return value;
	}

initsome(varis,num,isstatic,initnode)
	char *varis;
	int  num,isstatic,initnode; {
	int  asize,nodeo[4],inode,lasttree,swant;
	char *memat,sis,at,sinit,itype;

	do {
		if (*varis == ARRAY) {
			asize=(varis+1)->word;
			if (asize < 0 && num < 0) {
				error("missing dimension");
				return 0;
				}
			if (ifch('{')) {
				/*	allow the improper braces if string init of char */
				if (heir == STRNG && ((varis+3)->byte == CCHAR || (varis+3)->byte == CSCHAR))
					initsome(varis,1,isstatic,initnode);
				else initnode=initsome(varis+3,asize,isstatic,initnode);
				notch('}');
				}
			else if (heir == STRNG && ((varis+3)->byte == CCHAR || (varis+3)->byte == CSCHAR)) {
				if (isstatic == 0) {
					error("sorry, no string initilization of AUTO");
					return 0;
					}
				if (asize < 0)
					(varis+1)->word = 0;
concatstring:
				ctlb(INITSTR);
				sis=0;
				while(string[sis]) sis++;
				if (asize < 0) {
					(varis+1)->word+=sis;
					swant=sis;
					}
				else swant=asize-1;
				if (sis > swant) sis=swant;
				if (swant > 255) {
					error("string too long");
					swant=255;
					}
				ctlb(swant);
				i=0;
				while (i < sis)
					ctlb(string[i++]);
				while (sis++ < swant)
					ctlb(0);
				tokit();
				if(heir==STRNG) goto concatstring;
				ctlb(0xFF);
				if (asize < 0)
					(varis+1)->word++; // trailing zero
				}
			else {
				if (curch == ',' || curch == '}' || curch == ';') {
					if (num < 0) {
						bptr=varis-3;
						(bptr+1)->word=-1-num;
						}
					else {
						asize=dsize(varis)*num;
						if (isstatic) {
							ctlb(INITDB);
							ctlw(asize);
							}
						else locplus+=asize;
						}
					return initnode;
					}
				initnode=initsome(varis+3,asize,isstatic,initnode);
				}
			}
		else if (*varis == CSTRUCT) {
			memat=(varis+1)->word;
			memat=memat->schain;
			sinit=ifch('{');
			if ((curch == '}' || curch == ';') && num < 0) {
				bptr=varis-3;
				(bptr+1)->word=-1-num;
				return initnode;
				}
			while (memat) {
				initnode=initsome(&memat->ntype[0],1,isstatic,initnode);
				memat=memat->nchain;
				ifch(',');
				}
			if (sinit) notch('}');
			}

		else {
			if (curch == ',' || curch == '}' || curch == ';') {
				if (num < 0) {
					bptr=varis-3;
					while (*bptr != ARRAY)
						bptr--;
					(bptr+1)->word=-1-num;
					}
				else {
					asize=dsize(varis)*num;
					if (isstatic) {
						ctlb(INITDB);
						ctlw(asize);
						}
					else locplus+=asize;
					}
				return initnode;
				}
			if (ifch('{')) {
				initnode=initsome(varis,num,isstatic,initnode);
				notch('}');
				return initnode;
				}
			itype=*varis;
			if (itype == PTRTO && !is_big) itype=CUNSG;
			ininit=1;	/* needed so new variable is not allowed in init list*/
			if (isstatic) {
				lasttree=ntree;	/*	re-use space for each expression */
				inode=exprnc();
				ctlb(INITVAL);
				ctlb(itype);
				ctlw(lasttree-1);/*	supply place to chop off tree	*/
				ctlw(ntree-1);
				ctlw(inode);
				ntree=lasttree;	/*	chop of the tree	*/
				}
			else {
				nodeo[0]=typeof(varis);
				nodeo[1]=0;
				nodeo[2]=locoff+locplus;
				locplus+=dsize(varis);
				nodeo[3]=0;
				nodeo[1]=tree4(nodeo);
				nodeo[0]=ASGN;
				nodeo[2]=exprnc();
				if (initnode) {
					nodeo[2]=tree3(nodeo);
					nodeo[1]=initnode;
					nodeo[0]=LST+2;
					}
				initnode=tree3(nodeo);
				}
			ininit=0;
			}
		if (num != 1) ifch(',');
		}
	while (--num);
	return initnode;
	}



/*	FINDFILE.C	*/
/*
	This file contains the routine to locate a file, utilizing the INCLUDE
	environment variable for the directories to search.

	Interface:

		findfile(filename, target_buf)

		where filename is the name of the file to be searched for,
		      target_buf is the buffer to place the path name if found.

		if the file is found, findfile return 1 and the pathname,
		otherwise it returns 0.

	This program uses the environ routines to access the PATH variable.

	Stack requirements:  ~300 bytes

*/

findfile(filename, target_buf)
	char *filename, *target_buf; {
	int fid;
	char paths[256], *p_ptr, *t_ptr;

	/* first check in the local directory */
	strcpy(target_buf, filename);
	fid = open(target_buf, 0);
	if (fid >= 0)  {				/* got it */
		close(fid);
		return (1);
		}
	fid = _environ("DSINC", paths, 256);
	if (fid == -1) fid = _environ("INCLUDE", paths, 256);
	p_ptr = paths;

	while (*p_ptr != 0) {
		/* copy the directory name */
		t_ptr = target_buf;
		while (*p_ptr != ';' && *p_ptr != 0) {
			*t_ptr++ = *p_ptr++;
			}
		if (*(t_ptr-1) != '/' && *(t_ptr-1) != '\\') *t_ptr++ = '\\';
		*t_ptr = 0;
		if (*p_ptr) p_ptr++;		/* beyond the ';' */
		strcat(target_buf, filename);
		fid = open(target_buf, 0);
		if (fid >= 0)  {				/* got it */
			close(fid);
			return (1);
			}
		}
	strcpy(target_buf, filename);
	return (0);							/* can't find one */
	}



/*  ENVIRON.C	*/

/*
	This file contains the routine for searching the environment
	area for a given string.

	Interface:

		_environ(search_str, buffer, buf_len)

		where search_str is the actual string being searched for,
			  buffer is the location to store the string if found,
			  buf_len is the maximum number of characters to store
			          into the buffer (includes terminator).

        -1 is returned if the environment variable isn't found,
        otherwise the length of the string copied is returned.
*/

extern unsigned _pcb;		/* c88 variable with the PSP segment */

int _environ(search_str, buffer, buf_len)
	char *search_str, *buffer; int buf_len; {

#asm

	mov		ax, _pcb_;				; get the env segment into ES
	mov		es, ax					; 
	mov		ax, es:[2CH]
	mov		es,ax

	mov		bx, [6][bp]				; buffer
	mov		byte[bx],0
	mov		si, 0					; offset into the environment
	jmp		outer_begin

outer_loop:
	inc		si
outer_begin:
	mov		di, [4][bp]				; search_str
	cmp		byte es:[si],0
	jne		cmp_loop
									; not found
	mov		ax,-1
	pop		bp
	ret

cmp_loop:
	mov		al, byte es:[si]		; pick up the search character
	cmp		al,'='
	je		end_str
	cmp		al, byte[di]
	jne		outer_next
	inc		si
	inc		di
	jmp		cmp_loop

end_str:
	cmp		byte[di], 0
	jne		outer_next
									; got it, copy them bytes!
	mov		cx, [8][bp]				; buf_len
	inc		si						; beyond the '='

move_loop:
	mov		al, byte es:[si]
	or		al,al
	jz		now_return
	dec		cx
	jz		now_return
	mov		byte[bx],al
	inc		si
	inc		bx
	jmp		move_loop

now_return:
	mov		byte[bx],0
	mov		ax, [8][bp]
	sub		ax, cx
	pop		bp
	ret

outer_next:
	cmp		byte es:[si],0				; skip to the next entry
	je		outer_loop
	inc		si
	jmp		outer_next
#
}
