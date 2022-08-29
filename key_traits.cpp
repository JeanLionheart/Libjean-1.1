#ifndef __KTTS__
#define __KTTS__
namespace jean
{
    template <typename T>
    class identity//return the obj itself's value
    {
    public:
        T operator()(const T&val)
        {
            return val;
        }
    };

    class string_traits
    {
    public:
        long operator()(const char*s)
        {
            long r=0;
            for(int i=0;(s[i]!='\0')&&(i<=1000);i++){
                r=r*5+s[i]-'A';
            }
            return r;
        }
    };

    template <typename T,typename key_type>
    class byte_traits//traits key from first bytes
    {
        key_type*lp;
    public:
        byte_traits():lp(nullptr){}
        key_type operator()(const T&val)
        {
            lp=(key_type*)(&val);
            return *lp;
        }
        
    };
}
#endif