/*	DUMPO.C			dump ACTL or AINT or .OBJ file  */

#define MODEL	1
#include "OBJ.H"
#define EOF	26

int  infile,ofile;
char inbuf[650],name[20],*inin,*endin,popt;

union {int word; char byte;};

main(argc,argv)
	int  argc;
	char *argv[]; {

	/*	P option means print publics only	*/
	if (argc > 2) {
		if (toupper(*argv[2]) == 'P' || toupper(*(argv[2]+1)) == 'P') popt=1;
		else ofile=creat(argv[2]);
		}
	doopen(argv[1]);
	dofile();
	if (ofile) close(ofile);
	}

doopen(file)
	char *file; {
	char i;

	i=0;
	strcpy(name,file);
	while (*file && *file != '.')
		name[i++]=*file++;
	if (*file != '.') strcpy(&name[i],".O");
	if ((infile=open(name,0)) == -1) {
		os("Cannot open ");
		os(name);
		ocrlf();
		exit();
		}
	inin=endin=0;
	}

dofile() {
	char len,nout,tlen,temp_popt;

	checkin();

	while (*inin != EOF) {
		checkin();
		switch(*inin++) {
			case OEOF:		temp_popt=popt;
							popt=0;
							os("End of module");
							popt=temp_popt;
							break;
			case OSTATIC:
			case OPUBLIC:	if (*(inin-1) == OPUBLIC) os("Public  Name=");
							else os("Static  Name=");
							temp_popt=popt;
							if (*inin != '_') popt=0;
							while (*inin)
								oc(*inin++);
							inin++;
							os(" number=");
							onum(inin->word);
							inin+=2;
							if (temp_popt) ocrlf();
							popt=temp_popt;
							break;
			case ORESERVE:	os("Reserve  Number=");
							onum(inin->word);
							inin+=2;
							os(" Bytes=");
							onum(inin->word);
							inin+=2;
							break;
			case OLOCAL:	os("Local  Number=");
							onum(inin->word);
							inin+=2;
							os(" Offset=");
							onum(inin->word);
							inin+=2;
							break;
			case ODSEG:		os("In DSEG");
							break;
			case OCSEG:		os("In CSEG");
							break;
			case OESEG:		os("In ESEG");
							break;
			case OBIG:		os("In BIG");
							break;
			case ONAMEREL:	os("Namerel=");
							onum(inin->word);
							inin+=2;
							break;
			case OJUMPREL:	os("Jumprel=");
							onum(inin->word);
							inin+=2;
							break;
			case OLNAMEREL:	os("Lnamerel=");
							onum(inin->word);
							inin+=2;
							break;
			case OSEGPTR:	os("Segptr=");
							onum(inin->word);
							inin+=2;
							break;
			case OPTR:		os("Ptr=");
							onum(inin->word);
							inin+=2;
							break;
			case OPTYPE:	os("Ptype=");
							goto type;
			case OMTYPE:	os("Mtype=");
							goto type;
			case OLTYPE:	os("Ltype=");
type:
							while (*inin)
								oc(*inin++);
							inin++;
							oc(' ');
							onum(inin->word);
							oc(' ');
							inin+=2;
							tlen=*inin++;
							while (tlen--) {
								oc(' ');
								switch (*inin++) {
									case 1:	os("char");
											break;
									case 2:	os("int");
											break;
									case 3:	os("unsg");
											break;
									case 4:	os("long");
											break;
									case 5:	os("float");
											break;
									case 6:	os("double");
											break;
									case 8:	os("struct ");
											onum(inin->word);
											inin+=2;
											tlen-=2;
											break;
									case 253:os("fun");
											break;
									case 254:os("[");
											onum(inin->word);
											os("]");
											inin+=2;
											tlen-=2;
											break;
									case 255:os("ptr");
											break;
									default:os("mystery=");
											onum(*(inin-1));
									}
								}
							break;

			case OLINE:		os("Line=");
							onum(inin->word);
							inin+=2;
							break;
			case ONAME:		os("Name=");
							while (*inin)
								oc(*inin++);
							inin++;
							break;
			case OLNAME:	os("Lname=");
							while (*inin)
								oc(*inin++);
							inin++;
							break;

			default:		len=*(inin-1);
							if (len < 129) {
								os("Mystery=");
								onum(len);
								}
							else {
								len-=128;
								nout=0;
								while (len) {
									if ((nout & 15) == 0) {
										if (nout) ocrlf();
										os("Contents=");
										}
									ohnum(*inin++);
									len--;
									nout++;
									}
								}
			}
		ocrlf();
		}
	}


checkin() {
	char *temp;
	int  nin;

	if (inin+128 > endin) {
		temp=inbuf;
		while(inin < endin)
			*temp++=*inin++;
		nin=read(infile,temp,512);
		temp[nin]=EOF;
		inin=inbuf;
		endin=temp+512;
		}
	}

onum(num)
	int  num; {

	if (num < 0) {
		oc('-');
		num=-num;
		}
	if (num >= 9)
		onum(num/10);
	oc((num % 10)+'0');
	}

ohnum(num)
	char num; {

	onibble(num>>4);
	onibble(num & 15);
	oc(' ');
	}

onibble(nib)
	char nib; {

	if (nib > 9) oc(nib+55);
	else oc(nib+'0');
	}

ocrlf() {
	oc(13);
	oc(10);
	}

os(str)
	char *str; {
	
	while (*str)
		oc(*str++);
	}

oc(ch)
	char ch; {

	if (popt == 0) if (ofile) putc(ch,ofile); else putchar(ch);
	}
