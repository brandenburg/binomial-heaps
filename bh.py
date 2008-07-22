#!/usr/bin/env python
#
# Copyright (c) 2008, Bjoern B. Brandenburg <bbb [at] cs.unc.edu>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holder nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS  PROVIDED BY THE COPYRIGHT HOLDERS  AND CONTRIBUTORS "AS IS"
# AND ANY  EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED  TO, THE
# IMPLIED WARRANTIES  OF MERCHANTABILITY AND  FITNESS FOR A  PARTICULAR PURPOSE
# ARE  DISCLAIMED. IN NO  EVENT SHALL  THE COPYRIGHT  OWNER OR  CONTRIBUTORS BE
# LIABLE  FOR   ANY  DIRECT,  INDIRECT,  INCIDENTAL,   SPECIAL,  EXEMPLARY,  OR
# CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT   NOT  LIMITED  TO,  PROCUREMENT  OF
# SUBSTITUTE  GOODS OR SERVICES;  LOSS OF  USE, DATA,  OR PROFITS;  OR BUSINESS
# INTERRUPTION)  HOWEVER CAUSED  AND ON  ANY  THEORY OF  LIABILITY, WHETHER  IN
# CONTRACT,  STRICT  LIABILITY, OR  TORT  (INCLUDING  NEGLIGENCE OR  OTHERWISE)
# ARISING IN ANY  WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF  ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

"""An implementation of Binomial Heaps.

From Wikipedia: 
  A binomial heap is a heap similar to a binary heap but also supporting the
  operation of merging two heaps quickly. This is achieved by using a special
  tree structure.

  All of the following operations work in O(log n) time on a binomial heap with
  n elements: 
    - Insert a new element to the heap 
    - Find the element with minimum key
    - Delete the element with minimum key from the heap 
    - Decrease key of a given element 
    - Delete given element from the heap 
    - Merge two given heaps to one heap

  More details: http://en.wikipedia.org/wiki/Binomial_heap
"""

# provide special value that is not None, since None is a valid value
Nil = object()

class NodeRef(object):
    "Reference to a node in the heap. Used for decreasing keys and deletion."
    def __init__(self, node):
        self.ref = node
        self.in_tree = True

class Node(object):
    "Internal node of the heap. Don't use directly."

    def __init__(self, key, val=Nil):
        self.degree = 0
        self.parent = None
        self.next   = None
        self.child  = None
        self.key    = key
        self.ref    = NodeRef(self)
        if val is Nil:
            val = key
        self.val    = val

    def __str__(self):
        k = lambda x: str(x.key) if x else 'NIL'
        return '(%s, c:%s, n:%s)' % (k(self), k(self.child), k(self.next))

    def link(self, other):
        "Makes other a subtree of self."
        other.parent  = self
        other.next    = self.child
        self.child    = other
        self.degree  += 1

    @staticmethod
    def roots_merge(h1, h2):
        """Merge two lists of heap roots, sorted by degree.
        Returns the new head.
        """
        if not h1:
            return h2
        if not h2:
            return h1
        if h1.degree < h2.degree:
            h  = h1
            h1 = h.next
        else:
            h  = h2
            h2 = h2.next
        p = h
        while h2 and h1:
            if h1.degree < h2.degree:
                p.next = h1
                h1 = h1.next
            else:
                p.next = h2
                h2 = h2.next
            p = p.next
        if h2:
            p.next = h2
        else:
            p.next = h1
        return h

    @staticmethod
    def roots_reverse(h):
        """Reverse the heap root list. 
        Returns the new head. Also clears parent references.
        """
        if not h:
            return None
        tail = None
        next = h
        h.parent = None
        while h.next:
            next = h.next
            h.next = tail
            tail   = h
            h = next
            h.parent = None
        h.next = tail
        return h


class NegInfinity(object):
    "Negative infinity. Dummy class used during deletion."
    def __lt__(self, other):
        return True
    
    def __le__(self, other):
        return True

    def __gt__(self, other):
        return False

    def __ge__(self, other):
        return False

    def __str__(self):
        return '-Inf'

NegInf = NegInfinity()

class BinomialHeap(object):
    """From Wi
    """
    def __init__(self, lst=[]):
        self.head = None
        for x in lst:
            try:
                self.insert(x[0], x[1])
            except TypeError:
                self.insert(x)

    def insert(self, key, value=Nil):
        """Insert value in to the 
        """
        n = Node(key, value)
        self.__union(n)
        return n.ref
    
    def union(self, other):
        """Merge other into self. Returns None.
        Note: This is a destructive operation, other is an empty heap 
              afterwards.
        """
        h2, other.head = other.head, None
        self.__union(h2)
    
    def min(self):
        pos = self.__min()
        return pos[0].val if pos else None
    
    def extract_min(self):
        # find mininum
        pos = self.__min()
        if not pos:
            return None
        else:
            (x, prev) = pos
            # remove from list
            if prev:
                prev.next = x.next
            else:
                self.head = x.next
            kids =  Node.roots_reverse(x.child)
            self.__union(kids)
            x.ref.in_tree = False
            return x.val

    def delete(self, noderef):
        self.decrease(noderef, NegInf)
        self.extract_min()

    def decrease(self, noderef, key):
        assert noderef.in_tree
        assert noderef.ref.ref == noderef
        node = noderef.ref
        assert key <= node.key
        node.key = key
        cur    = node
        parent = cur.parent
        while parent and cur.key < parent.key:
            # need to bubble up
            # swap refs
            parent.ref.ref, cur.ref.ref = cur, parent
            parent.ref, cur.ref         = cur.ref, parent.ref
            # now swap keys and payload
            parent.key, cur.key         = cur.key, parent.key
            parent.val, cur.val         = cur.val, parent.val
            # step up
            cur    = parent
            parent = cur.parent

    def __nonzero__(self):
        return self.head != None

    def __iter__(self):
        return self

    def __iadd__(self, other):
        self.union(other)
        return self

    def next(self):
        if self.head:
            return self.extract_min()
        else:
            raise StopIteration
        
    def __min(self):
        if not self.head:
            return None
        min  = self.head
        min_prev = None
        prev = min
        cur  = min.next
        while cur:
            if min.key > cur.key:
                min = cur
                min_prev = prev
            prev = cur
            cur  = cur.next
        return (min, min_prev)

    def __union(self, h2):
        if not h2:
            # nothing to do
            return
        h1 = self.head
        if not h1:
            self.head = h2
            return
        h1 = Node.roots_merge(h1, h2)
        prev = None
        x    = h1
        next = x.next
        while next:
            if x.degree != next.degree or \
                    (next.next and next.next.degree == x.degree):
                prev = x
                x    = next
            elif x.key <= next.key:
                # x becomes the root of next
                x.next = next.next
                x.link(next)
            else: 
                # next becomes the root of x
                if not prev:
                    # update the "master" head
                    h1 = next
                else:
                    # just update previous link
                    prev.next = next
                next.link(x)
                # x is not toplevel anymore, update ref by advancing
                x = next
            next = x.next
        self.head = h1

def heap(lst=[]):
    """Create a new heap. lst should be a sequence of (key, value) pairs.
    Shortcut for BinomialHeap(lst)
    """
    return BinomialHeap(lst)

if __name__ == "__main__":
    tokens1 = [(24, 'all'), (16, 'star'), (9, 'true.\nSinging'), (7, 'clear'), 
               (25, 'praises'), (13, 'to'), (5, 'Heel'), 
               (6, 'voices\nRinging'), (26, 'thine.'), (21, 'shine\nCarolina'),
               (2, 'sound'), (20, 'radiance'), (12, 'N-C-U.\nHail'), 
               (10, "Carolina's"), (3, 'of'), (17, 'of'), 
               (23, 'gem.\nReceive'), (19, 'its'), (0, '\nHark'), 
               (22, 'priceless'), (4, 'Tar'), (1, 'the'), (8, 'and'), 
               (15, 'brightest'), (11, 'praises.\nShouting'), 
               (18, 'all\nClear'), (14, 'the')]
    tokens2 = [(113, 'Tar'), (124, 'Rah!'), (112, 'a'), (103, 'Heel'), 
               (104, "born\nI'm"), (122, 'Rah,'), (119, "Car'lina-lina\nRah,"),
               (117, 'Rah,'), (102, 'Tar'), (108, 'bred\nAnd'), (125, 'Rah!'), 
               (107, 'Heel'), (118, 'Rah,'), (111, "die\nI'm"), 
               (115, 'dead.\nSo'), (120, 'Rah,'), (121, "Car'lina-lina\nRah,"),
               (109, 'when'), (105, 'a'), (123, "Car'lina-lina\nRah!"), 
               (110, 'I'), (114, 'Heel'), (101, 'a'), (106, 'Tar'), 
               (100, "\nI'm"), (116, "it's")]
    h1 = heap(tokens1)
    h2 = heap(tokens2)
    h3 = heap()
    line = "\n==================================="
    h3.insert(90, line)
    h3.insert(-1, line)
    h3.insert(200, line)
    h3.insert(201, '\n\n')

    h1 += h2
    h1 += h3
    for x in h1:
        print x, 

