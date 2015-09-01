/*

  TNKernel real-time kernel

  Copyright © 2004, 2010 Yuri Tiomkin
  All rights reserved.

  Permission to use, copy, modify, and distribute this software in source
  and binary forms and its documentation for any purpose and without fee
  is hereby granted, provided that the above copyright notice appear
  in all copies and that both that copyright notice and this permission
  notice appear in supporting documentation.

  THIS SOFTWARE IS PROVIDED BY THE YURI TIOMKIN AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL YURI TIOMKIN OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.

*/

  /* ver 2.6  */

#include "tn.h"
#include "tn_utils.h"


#ifdef USE_MUTEXES

/*
     The ceiling protocol in ver 2.6 is more "lightweight" in comparison
   to previous versions.
     The code of ceiling protocol is derived from Vyacheslav Ovsiyenko version
*/


// L. Sha, R. Rajkumar, J. Lehoczky, Priority Inheritance Protocols: An Approach
// to Real-Time Synchronization, IEEE Transactions on Computers, Vol.39, No.9, 1990

//----------------------------------------------------------------------------
//  Structure's Field mutex->id_mutex have to be set to 0
//----------------------------------------------------------------------------
int tn_mutex_create(TN_MUTEX * mutex,
                    int attribute,
                    int ceil_priority)
{

#if TN_CHECK_PARAM
   if(mutex == NULL)
      return TERR_WRONG_PARAM;
   if(mutex->id_mutex != 0) //-- no recreation
      return TERR_WRONG_PARAM;
   if(attribute != TN_MUTEX_ATTR_CEILING && attribute != TN_MUTEX_ATTR_INHERIT)
      return TERR_WRONG_PARAM;
   if(attribute == TN_MUTEX_ATTR_CEILING &&
         (ceil_priority < 1 || ceil_priority > TN_NUM_PRIORITY - 2))
      return TERR_WRONG_PARAM;
#endif

   queue_reset(&(mutex->wait_queue));
   queue_reset(&(mutex->mutex_queue));
   queue_reset(&(mutex->lock_mutex_queue));

   mutex->attr          = attribute;
   mutex->holder        = NULL;
   mutex->ceil_priority = ceil_priority;
   mutex->cnt           = 0;
   mutex->id_mutex      = TN_ID_MUTEX;

   return TERR_NO_ERR;
}

//----------------------------------------------------------------------------
int tn_mutex_delete(TN_MUTEX * mutex)
{
   TN_INTSAVE_DATA

   CDLL_QUEUE * que;
   TN_TCB * task;

#if TN_CHECK_PARAM
   if(mutex == NULL)
      return TERR_WRONG_PARAM;
   if(mutex->id_mutex != TN_ID_MUTEX)
      return TERR_NOEXS;
#endif

   TN_CHECK_NON_INT_CONTEXT

   if(tn_curr_run_task != mutex->holder)
   {
      tn_enable_interrupt();
      return TERR_ILUSE;
   }

   //-- Remove all tasks(if any) from mutex's wait queue

   while(!is_queue_empty(&(mutex->wait_queue)))
   {
      tn_disable_interrupt();

      que  = queue_remove_head(&(mutex->wait_queue));
      task = get_task_by_tsk_queue(que);

    //-- If the task in system's blocked list, remove it

      if(task_wait_complete(task))
      {
         task->task_wait_rc = TERR_DLT;
         tn_enable_interrupt();
         tn_switch_context();
      }
   }

   if(tn_chk_irq_disabled() == 0)
      tn_disable_interrupt();

   if(mutex->holder != NULL)  //-- If the mutex is locked
   {
      do_unlock_mutex(mutex);
      queue_reset(&(mutex->mutex_queue));
   }
   mutex->id_mutex = 0; // Mutex not exists now

   tn_enable_interrupt();

   return TERR_NO_ERR;
}

//----------------------------------------------------------------------------
int tn_mutex_lock(TN_MUTEX * mutex, unsigned long timeout)
{
   TN_INTSAVE_DATA

#if TN_CHECK_PARAM
   if(mutex == NULL || timeout == 0)
      return TERR_WRONG_PARAM;
   if(mutex->id_mutex != TN_ID_MUTEX)
      return TERR_NOEXS;
#endif

   TN_CHECK_NON_INT_CONTEXT

   tn_disable_interrupt();

   if(tn_curr_run_task == mutex->holder) //-- Recursive locking not enabled
   {
      tn_enable_interrupt();
      return TERR_ILUSE;
   }

   if(mutex->attr == TN_MUTEX_ATTR_CEILING)
   {
      if(tn_curr_run_task->base_priority < mutex->ceil_priority) //-- base pri of task higher
      {
         tn_enable_interrupt();
         return TERR_ILUSE;
      }

      if(mutex->holder == NULL) //-- mutex not locked
      {
         mutex->holder = tn_curr_run_task;

         //-- Add mutex to task's locked mutexes queue

         queue_add_tail(&(tn_curr_run_task->mutex_queue), &(mutex->mutex_queue));

         //-- Ceiling protocol

         if(tn_curr_run_task->priority > mutex->ceil_priority)
            change_running_task_priority(tn_curr_run_task, mutex->ceil_priority);

         tn_enable_interrupt();
         return TERR_NO_ERR;
      }
      else //-- the mutex is already locked
      {
         //--- Task -> to the mutex wait queue

         task_curr_to_wait_action(&(mutex->wait_queue),
                                  TSK_WAIT_REASON_MUTEX_C,
                                  timeout);
         tn_enable_interrupt();
         tn_switch_context();

         return tn_curr_run_task->task_wait_rc;
      }
   }
   else if(mutex->attr == TN_MUTEX_ATTR_INHERIT)
   {
      if(mutex->holder == NULL) //-- mutex not locked
      {
         mutex->holder = tn_curr_run_task;

         queue_add_tail(&(tn_curr_run_task->mutex_queue), &(mutex->mutex_queue));

         tn_enable_interrupt();

         return TERR_NO_ERR;
      }
      else //-- the mutex is already locked
      {
            //-- Base priority inheritance protocol
            //-- if run_task curr priority higher holder's curr priority

         if(tn_curr_run_task->priority < mutex->holder->priority)
            set_current_priority(mutex->holder, tn_curr_run_task->priority);

         task_curr_to_wait_action(&(mutex->wait_queue),
                                  TSK_WAIT_REASON_MUTEX_I,
                                  timeout);
         tn_enable_interrupt();
         tn_switch_context();

         return tn_curr_run_task->task_wait_rc;
      }
   }

   tn_enable_interrupt(); //-- Never reach
   return TERR_NO_ERR;
}

//----------------------------------------------------------------------------
//  Try to lock mutex
//----------------------------------------------------------------------------
int tn_mutex_lock_polling(TN_MUTEX * mutex)
{
   TN_INTSAVE_DATA
   int rc;

#if TN_CHECK_PARAM
   if(mutex == NULL)
      return TERR_WRONG_PARAM;
   if(mutex->id_mutex != TN_ID_MUTEX)
      return TERR_NOEXS;
#endif

   TN_CHECK_NON_INT_CONTEXT

   tn_disable_interrupt();

   rc = TERR_NO_ERR;
   for(;;) //-- Single iteration loop
   {
      if(tn_curr_run_task == mutex->holder) //-- Recursive locking not enabled
      {
         rc = TERR_ILUSE;
         break;
      }
      if(mutex->attr == TN_MUTEX_ATTR_CEILING && //-- base pri of task higher
          tn_curr_run_task->base_priority < mutex->ceil_priority)
      {
         rc = TERR_ILUSE;
         break;
      }
      if(mutex->holder == NULL) //-- the mutex is not locked
      {
         mutex->holder = tn_curr_run_task;
         queue_add_tail(&(tn_curr_run_task->mutex_queue), &(mutex->mutex_queue));

         if(mutex->attr == TN_MUTEX_ATTR_CEILING)
         {
            //-- Ceiling protocol

            if(tn_curr_run_task->priority > mutex->ceil_priority)
               change_running_task_priority(tn_curr_run_task, mutex->ceil_priority);
         }
      }
      else //-- the mutex is already locked
      {
         rc = TERR_TIMEOUT;
      }
      break;
   }

   tn_enable_interrupt();

   return rc;
}

//----------------------------------------------------------------------------
int tn_mutex_unlock(TN_MUTEX * mutex)
{
   TN_INTSAVE_DATA

#if TN_CHECK_PARAM
   if(mutex == NULL)
      return TERR_WRONG_PARAM;
   if(mutex->id_mutex != TN_ID_MUTEX)
      return TERR_NOEXS;
#endif

   TN_CHECK_NON_INT_CONTEXT

   tn_disable_interrupt();

   //-- Unlocking is enabled only for the owner and already locked mutex

   if(tn_curr_run_task != mutex->holder)
   {
      tn_enable_interrupt();
      return TERR_ILUSE;
   }

   do_unlock_mutex(mutex);
   tn_enable_interrupt();
   tn_switch_context();

   return TERR_NO_ERR;
}

//----------------------------------------------------------------------------
//   Routines
//----------------------------------------------------------------------------
int do_unlock_mutex(TN_MUTEX * mutex)
{
   CDLL_QUEUE * curr_que;
   TN_MUTEX * tmp_mutex;
   TN_TCB * task;
   int pr;

   //-- Delete curr mutex from task's locked mutexes queue

   queue_remove_entry(&(mutex->mutex_queue));
   pr = tn_curr_run_task->base_priority;

   //---- No more mutexes, locked by the our task

   if(!is_queue_empty(&(tn_curr_run_task->mutex_queue)))
   {
      curr_que = tn_curr_run_task->mutex_queue.next;
      while(curr_que != &(tn_curr_run_task->mutex_queue))
      {
         tmp_mutex = get_mutex_by_mutex_queque(curr_que);

         if(tmp_mutex->attr == TN_MUTEX_ATTR_CEILING)
         {
            if(tmp_mutex->ceil_priority < pr)
               pr = tmp_mutex->ceil_priority;
         }
         else if(tmp_mutex->attr == TN_MUTEX_ATTR_INHERIT)
         {
            pr = find_max_blocked_priority(tmp_mutex, pr);
         }
         curr_que = curr_que->next;
      }
   }

   //-- Restore original priority

   if(pr != tn_curr_run_task->priority)
      change_running_task_priority(tn_curr_run_task, pr);


   //-- Check for the task(s) that want to lock the mutex

   if(is_queue_empty(&(mutex->wait_queue)))
   {
      mutex->holder = NULL;
      return TRUE;
   }

   //--- Now lock the mutex by the first task in the mutex queue

   curr_que = queue_remove_head(&(mutex->wait_queue));
   task     = get_task_by_tsk_queue(curr_que);
   mutex->holder = task;

   if(mutex->attr == TN_MUTEX_ATTR_CEILING &&
                             task->priority > mutex->ceil_priority)
      task->priority = mutex->ceil_priority;

   task_wait_complete(task);
   queue_add_tail(&(task->mutex_queue), &(mutex->mutex_queue));

   return TRUE;
}

//----------------------------------------------------------------------------
int find_max_blocked_priority(TN_MUTEX * mutex, int ref_priority)
{
   int priority;
   CDLL_QUEUE * curr_que;
   TN_TCB * task;

   priority = ref_priority;
   curr_que = mutex->wait_queue.next;
   while(curr_que != &(mutex->wait_queue))
   {
      task = get_task_by_tsk_queue(curr_que);
      if(task->priority < priority) //--  task priority is higher
         priority = task->priority;

      curr_que = curr_que->next;
   }

   return priority;
}

//----------------------------------------------------------------------------
#endif //-- USE_MUTEXES
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


