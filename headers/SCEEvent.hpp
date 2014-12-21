/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/******** FILE : Component.hpp ********/
/**************************************/


/***
 * Author : Paul Cook
 * Original code from : http://www.gamedev.net/page/resources/_/technical/general-programming/using-varadic-templates-for-a-signals-and-slots-implementation-in-c-r3782
 ***/


#include <vector>
#include <algorithm>

namespace  SCE {

    //Forward declaration of event class
    template<class... Args> class Event;


    class BaseDelegate {

    public:
        virtual void clear() = 0;
        virtual ~BaseDelegate(){ }
    };


    template<class... Args>
    class AbstractDelegate : public BaseDelegate {

    protected:

        virtual ~AbstractDelegate();

        friend class Event<Args...>;

        void add(Event<Args...> *s){ v.push_back(s); }
        void remove(Event<Args...> *s){ v.erase(std::remove(v.begin(), v.end(), s), v.end()); }
        virtual void clear() {
            for(auto i : v) i->disconnect(*this);
            v.clear();
        }

        virtual void call(Args... args) = 0;

        std::vector<Event<Args...>*> v;
    };


    template<class T, class... Args>
    class ConcreteDelegate : public AbstractDelegate<Args...> {

    public:

        ConcreteDelegate(T *t, void(T::*f)(Args...), Event<Args...> &s);

    private:

        ConcreteDelegate(const ConcreteDelegate&);
        void operator=(const ConcreteDelegate&);

        friend class Event<Args...>;

        virtual void call(Args... args){ (t->*f)(args...); }

        T *t;
        void(T::*f)(Args...);
    };

    template<class... Args>
    class Event {

    public:

        Event(){ }
        ~Event(){ for(auto i: v) i->remove(this); }

        void connect(AbstractDelegate<Args...> &s){ v.push_back(&s); s.add(this); }
        void disconnect(AbstractDelegate<Args...> &s){ v.erase(std::remove(v.begin(), v.end(), &s), v.end()); }

        void operator()(Args... args){ for(auto i: v) i->call(args...); }

    private:

        Event(const Event&);
        void operator=(const Event&);

        std::vector<AbstractDelegate<Args...>*> v;
    };

    template<class... Args>
    AbstractDelegate<Args...>::~AbstractDelegate() {
        for(auto i : v) i->disconnect(*this);
    }

    template<class T, class... Args>
    ConcreteDelegate<T, Args...>::ConcreteDelegate(T *t, void(T::*f)(Args...), Event<Args...> &s) : t(t), f(f) {
        s.connect(*this);
    }

    class Delegate {

    public:

        Delegate(){ }
        ~Delegate(){ for(auto i: v) delete i; }

        template<class T, class... Args>
        void connect(T *t, void(T::*f)(Args...), Event<Args...> &s){
            v.push_back(new ConcreteDelegate<T, Args...>(t, f, s));
        }

        void disconnectAll(){
            for(auto i : v){
                i->clear();
            }
        }

    private:

        Delegate(const Delegate&);
        void operator=(const Delegate&);

        std::vector<BaseDelegate*> v;
    };

}
