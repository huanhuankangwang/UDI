#ifndef H_SK_URL_PARSER
#define H_SK_URL_PARSER


#include <String.h>


namespace android
{
class WKURLParser
{
public:
	WKURLParser();
	virtual ~WKURLParser();
	bool parse(const char* url);
	void reset();
	bool mTrickable;
	bool mIsChannel;
	bool mIsBookmark;
	const char* getShiftUrl();
	const char* getLiveMultiUrl();
	const char* getLiveRtspUrl();
	const char* getBookmark();
	int getTimeShiftLength() { return mTimeShiftLength; };
	bool isMultiLive();
	unsigned long getMulticastIP();

	unsigned short getMulticastPort();
	bool parseIGMPUrl(const char* url);
private:
	String mLiveMultiUrl;
	String mLiveRtspUrl;
	String mShiftUrl;
	String bookmark;
	bool mIsMultiLive;

	unsigned long mMulticastIP;
	unsigned short mMulticastPort;

	int mTimeShiftLength;
};
}
#endif//:H_SK_URL_PARSER
