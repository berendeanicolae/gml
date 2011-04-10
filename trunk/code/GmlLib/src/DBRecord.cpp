#include "DBRecord.h"

bool GML::DB::RecordHash::CreateFromText(char *text)
{
	unsigned char	bValue;
	unsigned char	*p;
	unsigned int	poz;

	if (text==NULL)
		return false;
	p = &Hash.bValue[0];
	poz = 0;

	while (((*text)!=0) && (poz<32))
	{
		if (((*text)>='0') && ((*text)<='9'))
			bValue = (*text)-'0';
		else if (((*text)>='A') && ((*text)<='F'))
			bValue = (*text)-'A'+10;
		else if (((*text)>='a') && ((*text)<='f'))
			bValue = (*text)-'a'+10;
		else return false;
		if ((poz & 1)==0)
		{
			(*p) = bValue << 4;
		} else {
			(*p) |= bValue;
			p++;
		}
		poz++;
		text++;
	}
	return (bool)(((*text)==0) && (poz==32));
}
bool GML::DB::RecordHash::ToString(GML::Utils::GString &str)
{
	if (str.Set("")==false)
		return false;
	for (unsigned int tr=0;tr<16;tr++)
		if (str.AddFormated("%02X",Hash.bValue[tr])==false)
			return false;
	return true;
}
void GML::DB::RecordHash::Copy(GML::DB::RecordHash &rHash)
{
	memcpy(Hash.bValue,rHash.Hash.bValue,16);
}
void GML::DB::RecordHash::Reset()
{
	memset(Hash.bValue,0,16);
}