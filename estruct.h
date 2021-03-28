#ifndef ESTRUCT_H
#define ESTRUCT_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <initializer_list>
#include <type_traits>
#include <string>
#include <vector>
#include <list>
#include <map>

#define ESTRUCT_TYPE_DEFINE(...) void __serlize(EStructPacker &p_packer)const { p_packer.serlize(__VA_ARGS__); } void __deserlize(EStructUnPacker &p_packer){p_packer.deserlize(__VA_ARGS__); }


class EStructPacker
{
    template<bool, bool, bool, bool, bool>
    friend struct _EStructTool;

    struct _Data
    {
        char *data;
        char *data_end;
        char *mem_end;

        _Data()
        {
            data = (char *)std::malloc(256);
            data_end = data;
            mem_end = data + 256;
        }
        ~_Data()
        {
            std::free(data);
            data = nullptr;
            data_end = nullptr;
            mem_end = nullptr;
        }

        void expand()
        {
            size_t size = mem_end - data;
            size_t len = data_end - data;
            size = size * 2;
            data = (char *)std::realloc(data, size);
            data_end = data + len;
            mem_end = data + size;
        }

        void push(const void *p_data, size_t p_size)
        {
            size_t left_space = mem_end - data_end;
            while(left_space <= p_size)
            {
                expand();
                left_space = mem_end - data_end;
            }

            std::memcpy(data_end, p_data, p_size);
            data_end += p_size;
        }
    };

    _Data data;

public:

    template<class T>
    EStructPacker& _serlize(const T &p_data);

    inline EStructPacker& _serlize(const std::string &p_data);

    template<class T>
    EStructPacker& _serlize(const std::vector<T> &p_data);

    template<class T>
    EStructPacker& _serlize(const std::list<T> &p_data);

    template<class K, class V>
    EStructPacker& _serlize(const std::map<K, V> &p_data);

    template<class ...ARGS>
    EStructPacker& serlize(const ARGS &...p_args)
    {
        std::initializer_list<int>{(_serlize(p_args), 0)...};
        return (*this);
    }

    const void *ptr()const
    {
        return data.data;
    }

    size_t size()const
    {
        return data.mem_end - data.data;
    }
};

class EStructUnPacker
{
    template<bool, bool, bool, bool, bool>
    friend struct _EStructTool;

    struct _Data
    {
        const char *data;
        const char *data_tag;
        const char *data_end;

        void pop_data(void *p_data, size_t p_size)
        {
            size_t space = data_end - data_tag;
            assert(space >= p_size);

            memcpy(p_data, data_tag, p_size);
            data_tag += p_size;
        }

        const char *pop_string()
        {
            const char *res = data_tag;
            char c;
            while (true) 
            {
                c = *data_tag;
                data_tag++;
                if(c == 0)
                {
                    break;
                }
            }
            assert(data_end >= data_tag);
            return res;
        }
    };

    _Data data;
public:
    EStructUnPacker(const void *p_data, size_t p_size)
    {
        data.data = (const char *)p_data;
        data.data_tag = data.data;
        data.data_end = data.data + p_size;
    }

    template<class ...ARGS>
    EStructUnPacker& deserlize(ARGS &...args)
    {
        std::initializer_list<int>{(_deserlize(args), 0)...};
        return (*this);
    }

    template<class T>
    EStructUnPacker& _deserlize(T &p_data);

    inline EStructUnPacker& _deserlize(std::string &p_data);

    template<class T>
    EStructUnPacker& _deserlize(std::vector<T> &p_data);

    template<class T>
    EStructUnPacker& _deserlize(std::list<T> &p_data);

    template<class K, class V>
    EStructUnPacker& _deserlize(std::map<K, V> &p_data);
};

// (pod data) (is pointer) (is struct) (is array) (is std) 
template<bool, bool, bool, bool, bool>
struct _EStructTool{};

template<> // pod data
struct _EStructTool<true, false, false, false, false>
{
    template<class T>
    static void SerlizeData(EStructPacker& p_packer, const T&p_data)
    {
        p_packer.data.push(&p_data, sizeof(T));
    }

    template<class T>
    static void DeSerlizeData(EStructUnPacker& p_packer, T&p_data)
    {
        p_packer.data.pop_data(&p_data, sizeof(T));
    }
};

template<> // pointer
struct _EStructTool<true, false, true, false, false>
{
    // not support
};

template<> // pod struct
struct _EStructTool<true, true, false, false, false>
{
    template<class T>
    static void SerlizeData(EStructPacker& p_packer, const T&p_data)
    {
        p_packer.data.push(&p_data, sizeof(T));
    }

    template<class T>
    static void DeSerlizeData(EStructUnPacker& p_packer, T&p_data)
    {
        p_packer.data.pop_data(&p_data, sizeof(T));
    }
};

template<> // not pod struct
struct _EStructTool<false, false, true, false, false>
{
    template<class T>
    static void SerlizeData(EStructPacker& p_packer, const T&p_data)
    {
        p_data.__serlize(p_packer);
    }

    template<class T>
    static void DeSerlizeData(EStructUnPacker& p_packer, T&p_data)
    {
        p_data.__deserlize(p_packer);
    }
};

template<> // std
struct _EStructTool<false, false, true, false, true>
{
    static void SerlizeData(EStructPacker &p_packer, const std::string &p_data)
    {
        p_packer.data.push(p_data.c_str(), p_data.size() + 1);
    }

    static void DeSerlizeData(EStructUnPacker &p_packer, std::string &p_data)
    {
        p_data.clear();
        p_data = p_packer.data.pop_string();
    }

    template<class T>
    static void SerlizeData(EStructPacker &p_packer, const std::vector<T> &p_vec)
    {
        size_t s = p_vec.size();
        p_packer.data.push(&s, sizeof(s));
        for(size_t i = 0; i < s; ++i)
        {
            T &s_data = p_vec[i];
            p_packer.serlize(s_data);
        }
    }

    template<class T>
    static void DeSerlizeData(EStructUnPacker &p_packer, std::vector<T> &p_vec)
    {
        size_t s = 0;
        p_packer.data.pop_data(&s, sizeof(s));
        p_vec.clear();
        p_vec.resize(s);
        for(size_t i = 0; i < s; ++i)
        {
            T &data = p_vec[i];
            p_packer.deserlize(data);
        }
    }

    template<class T>
    static void SerlizeData(EStructPacker& p_packer, const std::list<T> &p_list)
    {
        size_t s = p_list.size();
        p_packer.data.push(&s, sizeof(s));
        typedef typename std::list<T>::const_iterator Iter;
        for(Iter iter = p_list.begin(); iter != p_list.end(); ++iter)
        {
            p_packer.serlize(*iter);
        }
    }

    template<class T>
    static void DeSerlizeData(EStructUnPacker &p_packer, std::list<T> &p_list)
    {
        size_t s = 0;
        p_packer.data.pop_data(&s, sizeof(s));
        p_list.clear();
        for(size_t i = 0; i < s; ++i)
        {
            T tmp;
            p_packer.deserlize(tmp);
            p_list.emplace_back(std::move(tmp));
        }
    }

    template<class K, class V>
    static void SerlizeData(EStructPacker& p_packer, const std::map<K, V> &p_map)
    {
        size_t s = p_map.size();
        p_packer.data.push(&s, sizeof(s));
        typedef typename std::map<K, V>::const_iterator Iter;
        for(Iter iter = p_map.begin(); iter != p_map.end(); ++iter)
        {
            const K &key = (*iter).first;
            const V &val = (*iter).second;

            p_packer.serlize(key, val);
        }
    };

    template<class K, class V>
    static void DeSerlizeData(EStructUnPacker& p_packer, std::map<K, V> &p_map)
    {
        size_t s = 0;
        p_packer.data.pop_data(&s, sizeof(s));
        p_map.clear();
        for(size_t i = 0; i < s; ++i)
        {
            K key;
            V val;
            p_packer.deserlize(key, val);
            p_map[key] = std::move(val);
        }
    }
};

template<> // pod array
struct _EStructTool<true, false, false, true, false>
{
    template<class T, size_t N>
    static void SerlizeData(EStructPacker& p_packer, const T (&p_data)[N])
    {
        p_packer.data.push(&p_data, N * sizeof(T));
    }

    template<class T, size_t N>
    static void DeSerlizeData(EStructUnPacker& p_packer, T (&p_data)[N])
    {
        p_packer.data.pop_data(&p_data, N * sizeof(T));
    }
};

template<> // not pod array
struct _EStructTool<false, false, false, true, false>
{
    template<class T, size_t N>
    static void SerlizeData(EStructPacker& p_packer, const T (&p_data)[N])
    {
        for(size_t i = 0; i < N; ++i)
        {
            p_packer.serlize(p_data[i]);
        }
    }

    template<class T, size_t N>
    static void DeSerlizeData(EStructUnPacker& p_packer, T (&p_data)[N])
    {
        for(size_t i = 0; i < N; ++i)
        {
            p_packer.deserlize(p_data[i]);
        }
    }
};

template<class T>
EStructPacker& EStructPacker::_serlize(const T &p_data)
{
    _EStructTool<std::is_pod<T>::value, 
        std::is_pointer<T>::value, 
        std::is_class<T>::value, 
        std::is_array<T>::value,
        false>::SerlizeData(*this, p_data);
    return (*this);
}

inline EStructPacker& EStructPacker::_serlize(const std::string &p_data)
{
    _EStructTool<false, false, true, false, true>::SerlizeData(*this, p_data);
    return (*this);
}

template<class T>
EStructPacker& EStructPacker::_serlize(const std::vector<T> &p_data)
{
    _EStructTool<false, false, true, false, true>::SerlizeData(*this, p_data);
    return (*this);
}

template<class T>
EStructPacker& EStructPacker::_serlize(const std::list<T> &p_data)
{
    _EStructTool<false, false, true, false, true>::SerlizeData(*this, p_data);
    return (*this);
}

template<class K, class V>
EStructPacker& EStructPacker::_serlize(const std::map<K, V> &p_data)
{
    _EStructTool<false, false, true, false, true>::SerlizeData(*this, p_data);
    return (*this);
}

template<class T>
EStructUnPacker& EStructUnPacker::_deserlize(T &p_data)
{
    _EStructTool<std::is_pod<T>::value, 
        std::is_pointer<T>::value, 
        std::is_class<T>::value,
        std::is_array<T>::value,
        false>::DeSerlizeData(*this, p_data);
    return (*this);
}

inline EStructUnPacker& EStructUnPacker::_deserlize(std::string &p_data)
{
    _EStructTool<false, false, true, false, true>::DeSerlizeData(*this, p_data);
    return (*this);
}

template<class T>
EStructUnPacker& EStructUnPacker::_deserlize(std::vector<T> &p_data)
{
    _EStructTool<false, false, true, false, true>::DeSerlizeData(*this, p_data);
    return (*this);
}

template<class T>
EStructUnPacker& EStructUnPacker::_deserlize(std::list<T> &p_data)
{
    _EStructTool<false, false, true, false, true>::DeSerlizeData(*this, p_data);
    return (*this);
}

template<class K, class V>
EStructUnPacker& EStructUnPacker::_deserlize(std::map<K, V> &p_data)
{
    _EStructTool<false, false, true, false, true>::DeSerlizeData(*this, p_data);
    return (*this);
}

#endif
