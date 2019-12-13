#ifndef _HTP_MANAGER_H_
#define _HTP_MANAGER_H_

class HtpManager
{
private:

public:
    static HtpManager& GetInstance()
    {
        static HtpManager instance;
        return instance;
    }
};

#endif // _HTP_MANAGER_H_