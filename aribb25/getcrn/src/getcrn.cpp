#include <iostream>
#include <winscard.h>
#include <tchar.h>
#include  <iomanip>

typedef std::basic_string<_TCHAR> tstring;

#if defined(UNICODE) || defined(_UNICODE)
#define tcout std::wcout
//#define locale_japan() {std::locale::global(locale("jananese"));}
#else
#define tcout std::cout
//#define locale_japan() {std::locale::global(locale("ja_JP.UTF-8"));}
#endif

#define hexf(fl, wd) std::showbase<<std::hex<<std::setfill(fl)<<std::setw(wd)
#define decf(fl, wd) std::noshowbase<<std::dec<<std::setfill(fl)<<std::setw(wd)

#define byte_order(in,out, size) \
	{ \
		uint8_t *w_in = (uint8_t *)in; \
		uint8_t *w_out = (uint8_t *)out; \
		for(int i=0, j=size-1; i<size; i++,j--){ \
			*(w_out+i) = *(w_in+j); \
		} \
	}

using namespace std;

static void card_byte_to_uint32(uint8_t* in, uint32_t* out) {

	uint8_t cd_6[8] = { 0,0,0,0,0,0,0,0 };
	uint8_t cd_2[2] = { 0,0 };
	byte_order(in, cd_6, sizeof(cd_6) - 2);
	byte_order(in + 6, cd_2, sizeof(cd_2));
	uint64_t cd = *((uint64_t*)cd_6) * 100000UL + *(uint16_t*)cd_2;

	*(out + 4) = cd % 10000UL;
	*(out + 3) = cd / 10000UL % 10000UL;
	*(out + 2) = cd / 100000000UL % 10000UL;
	*(out + 1) = cd / 1000000000000UL % 10000UL;
	*(out) = (uint32_t)(cd / 10000000000000000UL);
}

static int card_check(SCARDHANDLE Handle, const BYTE *command, BYTE *recv, DWORD *len)
{
	int r = 0;
	long ret;
	BYTE bcardid[8]{};
	uint32_t icardid[5]{};

	ret = SCardTransmit(Handle, SCARD_PCI_T1, command, 5, NULL, recv, len);
	if (ret == SCARD_S_SUCCESS) {
		if (*(recv+(*len - 2)) == 0x90 && *(recv+(*len - 1)) == 0x00) {
			if (*(recv+4) == 0x21 && *(recv+5) == 0x00) {
				uint16_t check = 0;
				for (int i = 8; i < 14; i += 2) {
					check ^= *((uint16_t*)&recv[i]);
				}
				memcpy(bcardid, recv + 8, 6);
				memcpy(bcardid + 6, &check, 2);
				card_byte_to_uint32(bcardid, icardid);
				tcout << "\t\t" << decf(_T('0'), 4) << icardid[0] << _T("-") << decf(_T('0'), 4) << icardid[1] << _T("-") << decf(_T('0'), 4) << icardid[2] << _T("-") << decf(_T('0'), 4) << icardid[3] << _T("-") << decf(_T('0'), 4) << icardid[4] << endl;
				r = 1;
			}
			else {
				tcout << _T("\t\tカード初期化コマンド エラー : ") << hexf(_T('0'), 2) << *(recv+4) << " " << hexf(_T('0'), 2) << *(recv+5) << endl;
				r = -1;
			}
		}
		else {
			// status word error
			r = -2;
		}
	}
	else {
		tcout << _T("\t\tSCardTransmit() エラー : ") << decf(_T('0'), 0) << ret << endl;
	}
	return r;
}

int main()
{
	SCARDCONTEXT Context = NULL;
	SCARDHANDLE Handle = NULL;
	DWORD protocol = 0;
	LPTSTR reader_name = NULL;
	DWORD len = SCARD_AUTOALLOCATE;
	size_t offset = 0;
	const BYTE initb1[] = { 0x80, 0x5e, 0x00, 0x00, 0x00 };
	const BYTE initb25[] = { 0x90, 0x30, 0x00, 0x00, 0x00 };
	BYTE recv[256]{};

	::locale::global(locale(""));

	long ret = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &Context);
	if (ret != SCARD_S_SUCCESS) {
		tcout << _T("リソースマネージャーコンテキスト 確立失敗") << endl;
		exit(0);
	}

	ret = SCardListReaders(Context, NULL, (LPTSTR)&reader_name, &len);
	if (ret != SCARD_S_SUCCESS) {
		tcout << _T("カードリーダー名 取得失敗") << endl;
		exit(0);
	}

	if (reader_name) {
		while (*(reader_name + offset) != _T('\0')) {
			tcout << reader_name + offset << endl;
			ret = SCardConnect(Context, reader_name + offset, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1, &Handle, &protocol);
			if (ret == SCARD_S_SUCCESS) {
				len = 256;
				ret = card_check(Handle, initb1, recv, &len);
				if (ret == -2) {
					len = 256;
					ret = card_check(Handle, initb25, recv, &len);
					if (ret == -2) {
						tcout << _T("\t\tカード初期化コマンド ステータスワード エラー : ") << hexf(_T('0'), 2) << recv[len - 2] << " " << hexf(_T('0'), 2) << recv[len - 1] << endl;
					}
				}
				SCardDisconnect(Handle, SCARD_LEAVE_CARD);
			}
			else {
				tcout << _T("\t\tSCardConnect() エラー : ") << decf(_T('0'), 0) << ret << endl;
			}
			offset += _tcslen(reader_name + offset) + 1;
		}
		SCardFreeMemory(Context, reader_name);
	}

	SCardReleaseContext(Context);

	exit(1);
}