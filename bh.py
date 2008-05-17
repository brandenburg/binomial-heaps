#!/usr/bin/env python

"""Binomial Heaps
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

    def all(self):
        s = lambda x: x.all() if x else []
        return [self.key] + s(self.child) + s(self.next)

    def __str__(self):
        #return '(%s, %s)' % (self.key, self.val)
        k = lambda x: str(x.key) if x else 'NIL'
        return '(%s, c:%s, n:%s)' % (k(self), k(self.child), k(self.next))

    def link(self, other):
        "Makes other a subtree of self."
        other.parent  = self
        other.next    = self.child
        self.child    = other
        self.degree  += 1


def _merge(h1, h2):
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

def _reverse(h):
    """Reverse the heap root list. 
       Returns the new head.
    """
    if not h:
        return None
    tail = None
    next = h
    while h.next:
        next = h.next
        h.next = tail
        tail   = h
        h = next
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
    def __init__(self, lst=[]):
        self.head = None
        for x in lst:
            self.insert(x)

    def insert(self, key, value=Nil):
        n = Node(key, value)
        self.__union(n)
        return n.ref
    
    def union(self, other):
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
            kids =  _reverse(x.child)
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
        self.insert(other)
        return self

    def __ior__(self, other):
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
        h1 = _merge(h1, h2)
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
    "Shortcut for BinomialHeap(lst)"
    return BinomialHeap(lst)
