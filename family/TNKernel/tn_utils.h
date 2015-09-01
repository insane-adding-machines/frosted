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

#ifndef  _TN_UTILS_H_
#define  _TN_UTILS_H_

  //-- Circular double-linked list queue --

#include "tn.h"

#ifdef __cplusplus
extern "C"  {
#endif

//________________________________________________________________
//
#if defined USE_INLINE_CDLL

INLINE_FORCED
void queue_reset(CDLL_QUEUE *que)
{
   que->prev = que->next = que;
}

INLINE_FORCED
BOOL is_queue_empty(CDLL_QUEUE *que)
{
    return (que->next == que);
}

INLINE_FORCED
void queue_remove_entry(CDLL_QUEUE *entry)
{
// The local vars helps compiler to do optimization - we know that
// after removing  the entry cannot point to itself
    CDLL_QUEUE *prev, *next;
//
//  TN_ASSERT(entry, "Attempt to remove NULL entry from CDLL");
//
    prev = entry->prev;
    next = entry->next;
    prev->next = next;
    next->prev = prev;
}

INLINE_FORCED
void queue_add_head(CDLL_QUEUE *que, CDLL_QUEUE *entry)
{
//
//  TN_ASSERT(entry, "Attempt to add NULL entry to CDLL");
//  TN_ASSERT(que,   "Attempt to add entry to NULL CDLL");
//
    entry->prev = que;
    entry->next = que->next;
    entry->next->prev = entry;
    que->next = entry;
}

INLINE_FORCED
void queue_add_tail(CDLL_QUEUE *que, CDLL_QUEUE *entry)
{
//
//  TN_ASSERT(entry, "Attempt to add NULL entry to CDLL");
//  TN_ASSERT(que,   "Attempt to add entry to NULL CDLL");
//
    entry->next = que;
    entry->prev = que->prev;
    entry->prev->next = entry;
    que->prev = entry;
}

INLINE_FORCED
CDLL_QUEUE* queue_remove_head(CDLL_QUEUE *que)
{
    CDLL_QUEUE *entry, *next;
//
//  TN_ASSERT(que, "Attempt to remove entry from NULL CDLL");
//
    entry = que->next;
    next  = entry->next;
    next->prev = que;
    que->next = next;
    return entry;
}

INLINE_FORCED
CDLL_QUEUE* queue_remove_tail(CDLL_QUEUE *que)
{
    CDLL_QUEUE *entry, *prev;
//
//  TN_ASSERT(que, "Attempt to remove entry from NULL CDLL");
//
    entry = que->prev;
    prev  = entry->prev;
    prev->next = que;
    que->prev = prev;
    return entry;
}
#else
void queue_reset(CDLL_QUEUE *que);
BOOL is_queue_empty(CDLL_QUEUE *que);
void queue_add_head(CDLL_QUEUE * que, CDLL_QUEUE * entry);
void queue_add_tail(CDLL_QUEUE * que, CDLL_QUEUE * entry);
CDLL_QUEUE * queue_remove_head(CDLL_QUEUE * que);
CDLL_QUEUE * queue_remove_tail(CDLL_QUEUE * que);
void queue_remove_entry(CDLL_QUEUE * entry);
#endif

BOOL queue_contains_entry(CDLL_QUEUE * que, CDLL_QUEUE * entry);

int  dque_fifo_write(TN_DQUE * dque, void * data_ptr);
int  dque_fifo_read(TN_DQUE * dque, void ** data_ptr);

#ifdef __cplusplus
}
#endif


#endif

