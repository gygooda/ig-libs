#include <unistd.h>

#include "log.h"
#include "config.h"
#include "utils.h"
#include "thread.h"
#include "timer.h"
#include "shared_queue.h"

using namespace LibSys;

class Tthread : public Thread
{
public:
    Tthread() : Thread()
    {}
    virtual ~Tthread()
    {}

private:
    virtual int thread_pre_run()
    {
        printf("pre run \n");
        return 0;
    }
    virtual int thread_post_run()
    {
        printf("post run \n");
        return 0;
    }

    virtual int thread_run()
    {
        for(int i = 0; i < 5; ++i)
        {
            printf("thread id: %lu, count: %d\n", thread_id(), i);
            sleep(1);
        }

        return 2;
    }
};

class Ttimer : public ITimerCallback
{
    public:
        Ttimer(){}
        virtual ~Ttimer(){}
        int timer_callback(void *arg)
        {
            static int count = 0;
            DEBUG_LOG("timer count: %d\n", count++);

            return 0;
        }
};

int main()
{
   LOGGER.set_file_name("test.log");
   LOGGER.set_log_level("DEBUG");

   CONFIG.load("test.ini", false);

   float f = CONFIG.get("f", 0.0f);
   int i = CONFIG.get("i", 0);
   const char *s = CONFIG.get("s", "def");

   DEBUG_LOG("f: %f, i: %d, s: %s", f, i, s);
   //LOGGER.rotate_log(NULL);
   //
   //unsigned int number = 127UL;
   //printf("power of 2: %lu\n", roundup_pow_of_two(number));

   /*
   Tthread t;
   t.run();
   void* r = NULL;

   t.join(&r);
   printf("thread exit code: %p\n", r);
   */

   /*
   TimerManager tm;
   tm.add_timer(1, new Ttimer(), NULL);
   for(int i = 0; i < 5; ++i)
   {
       tm.process_timer();
       sleep(1);
   }
   */

   /*
   SharedQueue sq;
   sq.init(32);

   sq.write("hello world!", sizeof("hello world!"));
   unsigned long readable_size = sq.get_readable_size();
   unsigned long writeable_size = sq.get_writeable_size();
   char buf[32] = {0};
   sq.read(buf, 30);
   LOG(DEBUG, "total: %lu, read: %lu, write: %lu, content: %s", sq.get_size(), readable_size, writeable_size, buf);
   */

   return 0;
}
