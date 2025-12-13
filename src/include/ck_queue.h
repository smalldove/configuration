/*
 * Copyright 2012-2015 Samy Al Bahra.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 * $FreeBSD: release/9.0.0/sys/sys/queue.h 221843 2011-05-13 15:49:23Z mdf $
 */

#ifndef CK_QUEUE_H
#define	CK_QUEUE_H

#include <ck_pr.h>

/*
 * This file defines three types of data structures: singly-linked lists,
 * singly-linked tail queues and lists.
 *
 * A singly-linked list is headed by a single forward pointer. The elements
 * are singly linked for minimum space and pointer manipulation overhead at
 * the expense of O(n) removal for arbitrary elements. New elements can be
 * added to the list after an existing element or at the head of the list.
 * Elements being removed from the head of the list should use the explicit
 * macro for this purpose for optimum efficiency. A singly-linked list may
 * only be traversed in the forward direction.  Singly-linked lists are ideal
 * for applications with large datasets and few or no removals or for
 * implementing a LIFO queue.
 *
 * A singly-linked tail queue is headed by a pair of pointers, one to the
 * head of the list and the other to the tail of the list. The elements are
 * singly linked for minimum space and pointer manipulation overhead at the
 * expense of O(n) removal for arbitrary elements. New elements can be added
 * to the list after an existing element, at the head of the list, or at the
 * end of the list. Elements being removed from the head of the tail queue
 * should use the explicit macro for this purpose for optimum efficiency.
 * A singly-linked tail queue may only be traversed in the forward direction.
 * Singly-linked tail queues are ideal for applications with large datasets
 * and few or no removals or for implementing a FIFO queue.
 *
 * A list is headed by a single forward pointer (or an array of forward
 * pointers for a hash table header). The elements are doubly linked
 * so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before
 * or after an existing element or at the head of the list. A list
 * may only be traversed in the forward direction.
 *
 * It is safe to use _FOREACH/_FOREACH_SAFE in the presence of concurrent
 * modifications to the list. Writers to these lists must, on the other hand,
 * implement writer-side synchronization. The _SWAP operations are not atomic.
 * This facility is currently unsupported on architectures such as the Alpha
 * which require load-depend memory fences.
 *
 *				CK_SLIST	CK_LIST	CK_STAILQ
 * _HEAD			+		+	+
 * _HEAD_INITIALIZER		+		+	+
 * _ENTRY			+		+	+
 * _INIT			+		+	+
 * _EMPTY			+		+	+
 * _FIRST			+		+	+
 * _NEXT			+		+	+
 * _FOREACH			+		+	+
 * _FOREACH_SAFE		+		+	+
 * _INSERT_HEAD			+		+	+
 * _INSERT_BEFORE		-		+	-
 * _INSERT_AFTER		+		+	+
 * _INSERT_TAIL			-		-	+
 * _REMOVE_AFTER		+		-	+
 * _REMOVE_HEAD			+		-	+
 * _REMOVE			+		+	+
 * _SWAP			+		+	+
 * _MOVE			+		+	+
 */

/*
 * Singly-linked List declarations.
 */

/* 
 * CK_SLIST_HEAD - 定义单链表头结构
 * @name: 链表头结构体名称
 * @type: 链表中元素的类型
 * 
 * 该宏定义一个单链表头结构，包含指向第一个元素的指针。
 */
#define	CK_SLIST_HEAD(name, type)						\
struct name {									\
	struct type *cslh_first;	/* first element */				\
}

/* 
 * CK_SLIST_HEAD_INITIALIZER - 单链表头初始化器
 * @head: 链表头结构体
 * 
 * 该宏用于静态初始化单链表头，将第一个元素指针设置为NULL。
 */
#define	CK_SLIST_HEAD_INITIALIZER(head)						\
	{ NULL }

/* 
 * CK_SLIST_ENTRY - 定义单链表条目结构
 * @type: 链表中元素的类型
 * 
 * 该宏定义一个单链表条目结构，包含指向下一个元素的指针。
 */
#define	CK_SLIST_ENTRY(type)							\
struct {									\
	struct type *csle_next;	/* next element */				\
}

/*
 * Singly-linked List functions.
 */

/* 
 * CK_SLIST_EMPTY - 检查单链表是否为空
 * @head: 链表头指针
 * 
 * 返回: 如果链表为空返回真，否则返回假。
 */
#define	CK_SLIST_EMPTY(head)							\
	(ck_pr_load_ptr(&(head)->cslh_first) == NULL)

/* 
 * CK_SLIST_FIRST - 获取单链表的第一个元素
 * @head: 链表头指针
 * 
 * 返回: 指向第一个元素的指针，如果链表为空则返回NULL。
 */
#define	CK_SLIST_FIRST(head)							\
	(ck_pr_load_ptr(&(head)->cslh_first))

/* 
 * CK_SLIST_NEXT - 获取单链表元素的下一个元素
 * @elm: 当前元素指针
 * @field: 链表条目字段名
 * 
 * 返回: 指向下一个元素的指针，如果当前元素是最后一个则返回NULL。
 */
#define	CK_SLIST_NEXT(elm, field)						\
	ck_pr_load_ptr(&((elm)->field.csle_next))

/* 
 * CK_SLIST_FOREACH - 遍历单链表
 * @var: 循环变量，指向当前元素
 * @head: 链表头指针
 * @field: 链表条目字段名
 * 
 * 该宏用于安全地遍历单链表中的所有元素。
 */
#define	CK_SLIST_FOREACH(var, head, field)					\
	for ((var) = CK_SLIST_FIRST((head));					\
	    (var);								\
	    (var) = CK_SLIST_NEXT((var), field))

/* 
 * CK_SLIST_FOREACH_FROM - 从指定位置开始遍历单链表
 * @var: 循环变量，指向当前元素（如果为NULL则从头开始）
 * @head: 链表头指针
 * @field: 链表条目字段名
 * 
 * 该宏用于从指定位置开始遍历单链表，如果var为NULL则从头开始。
 */
#define	CK_SLIST_FOREACH_FROM(var, head, field)					\
	for ((var) = ((var) != NULL ? (var) : CK_SLIST_FIRST((head)));		\
	    (var);								\
	    (var) = CK_SLIST_NEXT((var), field))

/* 
 * CK_SLIST_FOREACH_SAFE - 安全遍历单链表（允许在遍历时删除元素）
 * @var: 循环变量，指向当前元素
 * @head: 链表头指针
 * @field: 链表条目字段名
 * @tvar: 临时变量，用于保存下一个元素
 * 
 * 该宏用于安全地遍历单链表，允许在遍历过程中删除当前元素。
 */
#define	CK_SLIST_FOREACH_SAFE(var, head, field, tvar)				\
	for ((var) = CK_SLIST_FIRST(head);					\
	    (var) && ((tvar) = CK_SLIST_NEXT(var, field), 1);			\
	    (var) = (tvar))

/* 
 * CK_SLIST_FOREACH_PREVPTR - 通过前驱指针遍历单链表
 * @var: 循环变量，指向当前元素
 * @varp: 指向当前元素指针的指针
 * @head: 链表头指针
 * @field: 链表条目字段名
 * 
 * 该宏通过前驱指针遍历单链表，适用于需要修改前驱指针的场景。
 */
#define	CK_SLIST_FOREACH_PREVPTR(var, varp, head, field)			\
	for ((varp) = &(head)->cslh_first;					\
	    ((var) = ck_pr_load_ptr(varp)) != NULL;				\
	    (varp) = &(var)->field.csle_next)

/* 
 * CK_SLIST_INIT - 初始化单链表
 * @head: 链表头指针
 * 
 * 该宏用于动态初始化单链表，将第一个元素指针设置为NULL。
 */
#define	CK_SLIST_INIT(head) do {						\
	ck_pr_store_ptr(&(head)->cslh_first, NULL);				\
	ck_pr_fence_store();							\
} while (0)

/* 
 * CK_SLIST_INSERT_AFTER - 在指定元素后插入新元素
 * @a: 指定元素指针
 * @b: 要插入的新元素指针
 * @field: 链表条目字段名
 * 
 * 该宏在指定元素后插入新元素。
 */
#define	CK_SLIST_INSERT_AFTER(a, b, field) do {					\
	(b)->field.csle_next = (a)->field.csle_next;				\
	ck_pr_fence_store();							\
	ck_pr_store_ptr(&(a)->field.csle_next, b);				\
} while (0)

/* 
 * CK_SLIST_INSERT_HEAD - 在链表头部插入新元素
 * @head: 链表头指针
 * @elm: 要插入的新元素指针
 * @field: 链表条目字段名
 * 
 * 该宏在链表头部插入新元素。
 */
#define	CK_SLIST_INSERT_HEAD(head, elm, field) do {				\
	(elm)->field.csle_next = (head)->cslh_first;				\
	ck_pr_fence_store();							\
	ck_pr_store_ptr(&(head)->cslh_first, elm);				\
} while (0)

/* 
 * CK_SLIST_INSERT_PREVPTR - 通过前驱指针插入新元素
 * @prevp: 指向前驱元素next指针的指针
 * @slistelm: 要插入位置后的元素指针
 * @elm: 要插入的新元素指针
 * @field: 链表条目字段名
 * 
 * 该宏通过前驱指针在指定位置插入新元素。
 */
#define	CK_SLIST_INSERT_PREVPTR(prevp, slistelm, elm, field) do {		\
	(elm)->field.csle_next = (slistelm);					\
	ck_pr_fence_store();							\
	ck_pr_store_ptr(prevp, elm);						\
} while (0)

/* 
 * CK_SLIST_REMOVE_AFTER - 删除指定元素后的元素
 * @elm: 指定元素指针
 * @field: 链表条目字段名
 * 
 * 该宏删除指定元素后的元素。
 */
#define CK_SLIST_REMOVE_AFTER(elm, field) do {					\
	ck_pr_store_ptr(&(elm)->field.csle_next,				\
	    (elm)->field.csle_next->field.csle_next);				\
} while (0)

/* 
 * CK_SLIST_REMOVE - 从链表中删除指定元素
 * @head: 链表头指针
 * @elm: 要删除的元素指针
 * @type: 链表中元素的类型
 * @field: 链表条目字段名
 * 
 * 该宏从链表中删除指定元素。
 */
#define	CK_SLIST_REMOVE(head, elm, type, field) do {				\
	if ((head)->cslh_first == (elm)) {					\
		CK_SLIST_REMOVE_HEAD((head), field);				\
	} else {								\
		struct type *curelm = (head)->cslh_first;			\
		while (curelm->field.csle_next != (elm))			\
			curelm = curelm->field.csle_next;			\
		CK_SLIST_REMOVE_AFTER(curelm, field);				\
	}									\
} while (0)

/* 
 * CK_SLIST_REMOVE_HEAD - 删除链表头部元素
 * @head: 链表头指针
 * @field: 链表条目字段名
 * 
 * 该宏删除链表头部元素。
 */
#define	CK_SLIST_REMOVE_HEAD(head, field) do {					\
	ck_pr_store_ptr(&(head)->cslh_first,					\
		(head)->cslh_first->field.csle_next);				\
} while (0)

/* 
 * CK_SLIST_REMOVE_PREVPTR - 通过前驱指针删除元素
 * @prevp: 指向要删除元素的前驱指针
 * @elm: 要删除的元素指针
 * @field: 链表条目字段名
 * 
 * 该宏通过前驱指针删除指定元素。
 */
#define CK_SLIST_REMOVE_PREVPTR(prevp, elm, field) do {				\
	ck_pr_store_ptr(prevp, (elm)->field.csle_next);			\
} while (0)

/* 
 * CK_SLIST_MOVE - 将一个链表的内容移动到另一个链表
 * @head1: 目标链表头指针
 * @head2: 源链表头指针
 * @field: 链表条目字段名
 * 
 * 该宏将源链表的内容移动到目标链表。
 */
#define CK_SLIST_MOVE(head1, head2, field) do {					\
	ck_pr_store_ptr(&(head1)->cslh_first, (head2)->cslh_first);		\
} while (0)

/*
 * This operation is not applied atomically.
 */
#define CK_SLIST_SWAP(a, b, type) do {						\
	struct type *swap_first = (a)->cslh_first;				\
	(a)->cslh_first = (b)->cslh_first;					\
	(b)->cslh_first = swap_first;						\
} while (0)

/*
 * Singly-linked Tail queue declarations.
 */

/* 
 * CK_STAILQ_HEAD - 定义单链表尾队列头结构
 * @name: 队列头结构体名称
 * @type: 队列中元素的类型
 * 
 * 该宏定义一个单链表尾队列头结构，包含指向第一个元素的指针和指向最后一个元素next指针的指针。
 */
#define	CK_STAILQ_HEAD(name, type)					\
struct name {								\
	struct type *cstqh_first;/* first element */			\
	struct type **cstqh_last;/* addr of last next element */		\
}

/* 
 * CK_STAILQ_HEAD_INITIALIZER - 单链表尾队列头初始化器
 * @head: 队列头结构体
 * 
 * 该宏用于静态初始化单链表尾队列头，将第一个元素指针设置为NULL，最后一个元素next指针指向第一个元素指针。
 */
#define	CK_STAILQ_HEAD_INITIALIZER(head)				\
	{ NULL, &(head).cstqh_first }

/* 
 * CK_STAILQ_ENTRY - 定义单链表尾队列条目结构
 * @type: 队列中元素的类型
 * 
 * 该宏定义一个单链表尾队列条目结构，包含指向下一个元素的指针。
 */
#define	CK_STAILQ_ENTRY(type)						\
struct {								\
	struct type *cstqe_next;	/* next element */			\
}

/*
 * Singly-linked Tail queue functions.
 */

/* 
 * CK_STAILQ_CONCAT - 连接两个单链表尾队列
 * @head1: 目标队列头指针（连接后包含两个队列的所有元素）
 * @head2: 源队列头指针（连接后被清空）
 * 
 * 该宏将源队列连接到目标队列的尾部，然后清空源队列。
 */
#define	CK_STAILQ_CONCAT(head1, head2) do {					\
	if ((head2)->cstqh_first != NULL) {					\
		ck_pr_store_ptr((head1)->cstqh_last, (head2)->cstqh_first);	\
		ck_pr_fence_store();						\
		(head1)->cstqh_last = (head2)->cstqh_last;			\
		CK_STAILQ_INIT((head2));					\
	}									\
} while (0)

/* 
 * CK_STAILQ_EMPTY - 检查单链表尾队列是否为空
 * @head: 队列头指针
 * 
 * 返回: 如果队列为空返回真，否则返回假。
 */
#define	CK_STAILQ_EMPTY(head)	(ck_pr_load_ptr(&(head)->cstqh_first) == NULL)

/* 
 * CK_STAILQ_FIRST - 获取单链表尾队列的第一个元素
 * @head: 队列头指针
 * 
 * 返回: 指向第一个元素的指针，如果队列为空则返回NULL。
 */
#define	CK_STAILQ_FIRST(head)	(ck_pr_load_ptr(&(head)->cstqh_first))

/* 
 * CK_STAILQ_FOREACH - 遍历单链表尾队列
 * @var: 循环变量，指向当前元素
 * @head: 队列头指针
 * @field: 队列条目字段名
 * 
 * 该宏用于安全地遍历单链表尾队列中的所有元素。
 */
#define	CK_STAILQ_FOREACH(var, head, field)				\
	for((var) = CK_STAILQ_FIRST((head));				\
	   (var);							\
	   (var) = CK_STAILQ_NEXT((var), field))

/* 
 * CK_STAILQ_FOREACH_FROM - 从指定位置开始遍历单链表尾队列
 * @var: 循环变量，指向当前元素（如果为NULL则从头开始）
 * @head: 队列头指针
 * @field: 队列条目字段名
 * 
 * 该宏用于从指定位置开始遍历单链表尾队列，如果var为NULL则从头开始。
 */
#define	CK_STAILQ_FOREACH_FROM(var, head, field)			\
	for ((var) = ((var) != NULL ? (var) : CK_STAILQ_FIRST((head)));	\
	    (var);							\
	    (var) = CK_STAILQ_NEXT((var), field))

/* 
 * CK_STAILQ_FOREACH_SAFE - 安全遍历单链表尾队列（允许在遍历时删除元素）
 * @var: 循环变量，指向当前元素
 * @head: 队列头指针
 * @field: 队列条目字段名
 * @tvar: 临时变量，用于保存下一个元素
 * 
 * 该宏用于安全地遍历单链表尾队列，允许在遍历过程中删除当前元素。
 */
#define	CK_STAILQ_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = CK_STAILQ_FIRST((head));				\
	    (var) && ((tvar) =						\
		CK_STAILQ_NEXT((var), field), 1);			\
	    (var) = (tvar))

/* 
 * CK_STAILQ_INIT - 初始化单链表尾队列
 * @head: 队列头指针
 * 
 * 该宏用于动态初始化单链表尾队列，将第一个元素指针设置为NULL，最后一个元素next指针指向第一个元素指针。
 */
#define	CK_STAILQ_INIT(head) do {					\
	ck_pr_store_ptr(&(head)->cstqh_first, NULL);			\
	ck_pr_fence_store();						\
	(head)->cstqh_last = &(head)->cstqh_first;			\
} while (0)

/* 
 * CK_STAILQ_INSERT_AFTER - 在指定元素后插入新元素
 * @head: 队列头指针
 * @tqelm: 指定元素指针
 * @elm: 要插入的新元素指针
 * @field: 队列条目字段名
 * 
 * 该宏在指定元素后插入新元素，如果新元素是最后一个元素，则更新队列尾指针。
 */
#define	CK_STAILQ_INSERT_AFTER(head, tqelm, elm, field) do {			\
	(elm)->field.cstqe_next = (tqelm)->field.cstqe_next;			\
	ck_pr_fence_store();							\
	ck_pr_store_ptr(&(tqelm)->field.cstqe_next, elm);			\
	if ((elm)->field.cstqe_next == NULL)					\
		(head)->cstqh_last = &(elm)->field.cstqe_next;			\
} while (0)

/* 
 * CK_STAILQ_INSERT_HEAD - 在队列头部插入新元素
 * @head: 队列头指针
 * @elm: 要插入的新元素指针
 * @field: 队列条目字段名
 * 
 * 该宏在队列头部插入新元素，如果队列原来为空，则更新队列尾指针。
 */
#define	CK_STAILQ_INSERT_HEAD(head, elm, field) do {				\
	(elm)->field.cstqe_next = (head)->cstqh_first;				\
	ck_pr_fence_store();							\
	ck_pr_store_ptr(&(head)->cstqh_first, elm);				\
	if ((elm)->field.cstqe_next == NULL)					\
		(head)->cstqh_last = &(elm)->field.cstqe_next;			\
} while (0)

/* 
 * CK_STAILQ_INSERT_TAIL - 在队列尾部插入新元素
 * @head: 队列头指针
 * @elm: 要插入的新元素指针
 * @field: 队列条目字段名
 * 
 * 该宏在队列尾部插入新元素，更新队列尾指针。
 */
#define	CK_STAILQ_INSERT_TAIL(head, elm, field) do {				\
	(elm)->field.cstqe_next = NULL;						\
	ck_pr_fence_store();							\
	ck_pr_store_ptr((head)->cstqh_last, (elm));				\
	(head)->cstqh_last = &(elm)->field.cstqe_next;				\
} while (0)

/* 
 * CK_STAILQ_NEXT - 获取单链表尾队列元素的下一个元素
 * @elm: 当前元素指针
 * @field: 队列条目字段名
 * 
 * 返回: 指向下一个元素的指针，如果当前元素是最后一个则返回NULL。
 */
#define	CK_STAILQ_NEXT(elm, field)						\
	(ck_pr_load_ptr(&(elm)->field.cstqe_next))

/* 
 * CK_STAILQ_REMOVE - 从队列中删除指定元素
 * @head: 队列头指针
 * @elm: 要删除的元素指针
 * @type: 队列中元素的类型
 * @field: 队列条目字段名
 * 
 * 该宏从队列中删除指定元素。
 */
#define	CK_STAILQ_REMOVE(head, elm, type, field) do {				\
	if ((head)->cstqh_first == (elm)) {					\
		CK_STAILQ_REMOVE_HEAD((head), field);				\
	} else {								\
		struct type *curelm = (head)->cstqh_first;			\
		while (curelm->field.cstqe_next != (elm))			\
			curelm = curelm->field.cstqe_next;			\
		CK_STAILQ_REMOVE_AFTER(head, curelm, field);			\
	}									\
} while (0)

/* 
 * CK_STAILQ_REMOVE_AFTER - 删除指定元素后的元素
 * @head: 队列头指针
 * @elm: 指定元素指针
 * @field: 队列条目字段名
 * 
 * 该宏删除指定元素后的元素，如果删除的是最后一个元素，则更新队列尾指针。
 */
#define CK_STAILQ_REMOVE_AFTER(head, elm, field) do {				\
	ck_pr_store_ptr(&(elm)->field.cstqe_next,				\
	    (elm)->field.cstqe_next->field.cstqe_next);				\
	if ((elm)->field.cstqe_next == NULL)					\
		(head)->cstqh_last = &(elm)->field.cstqe_next;			\
} while (0)

/* 
 * CK_STAILQ_REMOVE_HEAD - 删除队列头部元素
 * @head: 队列头指针
 * @field: 队列条目字段名
 * 
 * 该宏删除队列头部元素，如果删除后队列为空，则更新队列尾指针。
 */
#define	CK_STAILQ_REMOVE_HEAD(head, field) do {					\
	ck_pr_store_ptr(&(head)->cstqh_first,					\
	    (head)->cstqh_first->field.cstqe_next);				\
	if ((head)->cstqh_first == NULL)						\
		(head)->cstqh_last = &(head)->cstqh_first;			\
} while (0)

/* 
 * CK_STAILQ_MOVE - 将一个队列的内容移动到另一个队列
 * @head1: 目标队列头指针
 * @head2: 源队列头指针
 * @field: 队列条目字段名
 * 
 * 该宏将源队列的内容移动到目标队列。
 */
#define CK_STAILQ_MOVE(head1, head2, field) do {				\
	ck_pr_store_ptr(&(head1)->cstqh_first, (head2)->cstqh_first);		\
	(head1)->cstqh_last = (head2)->cstqh_last;				\
	if ((head2)->cstqh_last == &(head2)->cstqh_first)				\
		(head1)->cstqh_last = &(head1)->cstqh_first;			\
} while (0)

/*
 * This operation is not applied atomically.
 */

/* 
 * CK_STAILQ_SWAP - 交换两个单链表尾队列的内容
 * @head1: 第一个队列头指针
 * @head2: 第二个队列头指针
 * @type: 队列中元素的类型
 * 
 * 注意：此操作不是原子操作。
 * 该宏交换两个队列的内容，包括第一个元素指针和尾指针。
 */
#define CK_STAILQ_SWAP(head1, head2, type) do {				\
	struct type *swap_first = CK_STAILQ_FIRST(head1);		\
	struct type **swap_last = (head1)->cstqh_last;			\
	CK_STAILQ_FIRST(head1) = CK_STAILQ_FIRST(head2);		\
	(head1)->cstqh_last = (head2)->cstqh_last;			\
	CK_STAILQ_FIRST(head2) = swap_first;				\
	(head2)->cstqh_last = swap_last;					\
	if (CK_STAILQ_EMPTY(head1))					\
		(head1)->cstqh_last = &(head1)->cstqh_first;		\
	if (CK_STAILQ_EMPTY(head2))					\
		(head2)->cstqh_last = &(head2)->cstqh_first;		\
} while (0)

/*
 * List declarations.
 */

/* 
 * CK_LIST_HEAD - 定义双向链表头结构
 * @name: 链表头结构体名称
 * @type: 链表中元素的类型
 * 
 * 该宏定义一个双向链表头结构，包含指向第一个元素的指针。
 */
#define	CK_LIST_HEAD(name, type)						\
struct name {									\
	struct type *clh_first;	/* first element */				\
}

/* 
 * CK_LIST_HEAD_INITIALIZER - 双向链表头初始化器
 * @head: 链表头结构体
 * 
 * 该宏用于静态初始化双向链表头，将第一个元素指针设置为NULL。
 */
#define	CK_LIST_HEAD_INITIALIZER(head)						\
	{ NULL }

/* 
 * CK_LIST_ENTRY - 定义双向链表条目结构
 * @type: 链表中元素的类型
 * 
 * 该宏定义一个双向链表条目结构，包含指向下一个元素的指针和指向前一个元素next指针的指针。
 */
#define	CK_LIST_ENTRY(type)							\
struct {									\
	struct type *cle_next;	/* next element */				\
	struct type **cle_prev;	/* address of previous next element */		\
}

/* 
 * CK_LIST_FIRST - 获取双向链表的第一个元素
 * @head: 链表头指针
 * 
 * 返回: 指向第一个元素的指针，如果链表为空则返回NULL。
 */
#define	CK_LIST_FIRST(head)		ck_pr_load_ptr(&(head)->clh_first)
/* 
 * CK_LIST_EMPTY - 检查双向链表是否为空
 * @head: 链表头指针
 * 
 * 返回: 如果链表为空返回真，否则返回假。
 */
#define	CK_LIST_EMPTY(head)		(CK_LIST_FIRST(head) == NULL)
/* 
 * CK_LIST_NEXT - 获取双向链表元素的下一个元素
 * @elm: 当前元素指针
 * @field: 链表条目字段名
 * 
 * 返回: 指向下一个元素的指针，如果当前元素是最后一个则返回NULL。
 */
#define	CK_LIST_NEXT(elm, field)	ck_pr_load_ptr(&(elm)->field.cle_next)

/* 
 * CK_LIST_FOREACH - 遍历双向链表
 * @var: 循环变量，指向当前元素
 * @head: 链表头指针
 * @field: 链表条目字段名
 * 
 * 该宏用于安全地遍历双向链表中的所有元素。
 */
#define	CK_LIST_FOREACH(var, head, field)					\
	for ((var) = CK_LIST_FIRST((head));					\
	    (var);								\
	    (var) = CK_LIST_NEXT((var), field))

/* 
 * CK_LIST_FOREACH_FROM - 从指定位置开始遍历双向链表
 * @var: 循环变量，指向当前元素（如果为NULL则从头开始）
 * @head: 链表头指针
 * @field: 链表条目字段名
 * 
 * 该宏用于从指定位置开始遍历双向链表，如果var为NULL则从头开始。
 */
#define	CK_LIST_FOREACH_FROM(var, head, field)					\
	for ((var) = ((var) != NULL ? (var) : CK_LIST_FIRST((head)));		\
	    (var);								\
	    (var) = CK_LIST_NEXT((var), field))

/* 
 * CK_LIST_FOREACH_SAFE - 安全遍历双向链表（允许在遍历时删除元素）
 * @var: 循环变量，指向当前元素
 * @head: 链表头指针
 * @field: 链表条目字段名
 * @tvar: 临时变量，用于保存下一个元素
 * 
 * 该宏用于安全地遍历双向链表，允许在遍历过程中删除当前元素。
 */
#define	CK_LIST_FOREACH_SAFE(var, head, field, tvar)				  \
	for ((var) = CK_LIST_FIRST((head));					  \
	    (var) && ((tvar) = CK_LIST_NEXT((var), field), 1);			  \
	    (var) = (tvar))

/* 
 * CK_LIST_INIT - 初始化双向链表
 * @head: 链表头指针
 * 
 * 该宏用于动态初始化双向链表，将第一个元素指针设置为NULL。
 */
#define	CK_LIST_INIT(head) do {							\
	ck_pr_store_ptr(&(head)->clh_first, NULL);				\
	ck_pr_fence_store();							\
} while (0)

/* 
 * CK_LIST_INSERT_AFTER - 在指定元素后插入新元素
 * @listelm: 指定元素指针
 * @elm: 要插入的新元素指针
 * @field: 链表条目字段名
 * 
 * 该宏在指定元素后插入新元素，更新前后元素的指针。
 */
#define	CK_LIST_INSERT_AFTER(listelm, elm, field) do {				\
	(elm)->field.cle_next = (listelm)->field.cle_next;			\
	(elm)->field.cle_prev = &(listelm)->field.cle_next;			\
	ck_pr_fence_store();							\
	if ((listelm)->field.cle_next != NULL)					\
		(listelm)->field.cle_next->field.cle_prev = &(elm)->field.cle_next;\
	ck_pr_store_ptr(&(listelm)->field.cle_next, elm);			\
} while (0)

/* 
 * CK_LIST_INSERT_BEFORE - 在指定元素前插入新元素
 * @listelm: 指定元素指针
 * @elm: 要插入的新元素指针
 * @field: 链表条目字段名
 * 
 * 该宏在指定元素前插入新元素，更新前后元素的指针。
 */
#define	CK_LIST_INSERT_BEFORE(listelm, elm, field) do {				\
	(elm)->field.cle_prev = (listelm)->field.cle_prev;			\
	(elm)->field.cle_next = (listelm);					\
	ck_pr_fence_store();							\
	ck_pr_store_ptr((listelm)->field.cle_prev, (elm));			\
	(listelm)->field.cle_prev = &(elm)->field.cle_next;			\
} while (0)

/* 
 * CK_LIST_INSERT_HEAD - 在链表头部插入新元素
 * @head: 链表头指针
 * @elm: 要插入的新元素指针
 * @field: 链表条目字段名
 * 
 * 该宏在链表头部插入新元素，更新前后元素的指针。
 */
#define	CK_LIST_INSERT_HEAD(head, elm, field) do {				\
	(elm)->field.cle_next = (head)->clh_first;				\
	ck_pr_fence_store();							\
	if ((elm)->field.cle_next != NULL)					\
		(head)->clh_first->field.cle_prev =  &(elm)->field.cle_next;	\
	ck_pr_store_ptr(&(head)->clh_first, elm);				\
	(elm)->field.cle_prev = &(head)->clh_first;				\
} while (0)

/* 
 * CK_LIST_REMOVE - 从链表中删除指定元素
 * @elm: 要删除的元素指针
 * @field: 链表条目字段名
 * 
 * 该宏从链表中删除指定元素，更新前后元素的指针。
 */
#define	CK_LIST_REMOVE(elm, field) do {						\
	ck_pr_store_ptr((elm)->field.cle_prev, (elm)->field.cle_next);		\
	if ((elm)->field.cle_next != NULL)					\
		(elm)->field.cle_next->field.cle_prev = (elm)->field.cle_prev;	\
} while (0)

/* 
 * CK_LIST_MOVE - 将一个链表的内容移动到另一个链表
 * @head1: 目标链表头指针
 * @head2: 源链表头指针
 * @field: 链表条目字段名
 * 
 * 该宏将源链表的内容移动到目标链表。
 */
#define CK_LIST_MOVE(head1, head2, field) do {				\
	ck_pr_store_ptr(&(head1)->clh_first, (head2)->clh_first);		\
	if ((head1)->clh_first != NULL)					\
		(head1)->clh_first->field.cle_prev = &(head1)->clh_first;	\
} while (0)

/*
 * This operation is not applied atomically.
 */

/* 
 * CK_LIST_SWAP - 交换两个双向链表的内容
 * @head1: 第一个链表头指针
 * @head2: 第二个链表头指针
 * @type: 链表中元素的类型
 * @field: 链表条目字段名
 * 
 * 注意：此操作不是原子操作。
 * 该宏交换两个链表的内容，包括第一个元素指针和前后元素的指针。
 */
#define CK_LIST_SWAP(head1, head2, type, field) do {			\
	struct type *swap_tmp = (head1)->clh_first;			\
	(head1)->clh_first = (head2)->clh_first;				\
	(head2)->clh_first = swap_tmp;					\
	if ((swap_tmp = (head1)->clh_first) != NULL)			\
		swap_tmp->field.cle_prev = &(head1)->clh_first;		\
	if ((swap_tmp = (head2)->clh_first) != NULL)			\
		swap_tmp->field.cle_prev = &(head2)->clh_first;		\
} while (0)

#endif /* CK_QUEUE_H */
