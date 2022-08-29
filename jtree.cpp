#ifndef __TREES__
#define __TREES__
#include <iostream>
#include <queue>
#include <atomic>
#include <mutex>
#include "../jallocator/jallocator_t.cpp"
#include "../jallocator/jalloc_conc.cpp"
#include<pthread.h>
namespace jean
{
    /*thread safe, avl_tree */
    template <typename T, template <typename N> class jalloc_t = jalloc_conc>
    class jtree_avl
    { // thread-safe
    private:
        /* data */
        jalloc_t<T> data_jalloc;
        struct node
        { /*  */
            node *lc;
            node *rc;
            int lh, rh;
            long key;
            T *val;
            node(long k) : val(), lc(0), rc(0), lh(0), rh(0), key(k)
            {
            }
        };
        pthread_rwlock_t prw;
        void operator=(jtree_avl<T, jalloc_t> &);
        jtree_avl(jtree_avl<T, jalloc_t> &);

        jalloc_t<node> jalloc;
        node *create_node(long k, T &v)
        {
            node *res = (node *)jalloc.allocate();
            if (!res)
                return nullptr;
            new (res) node(k);
            res->val = (T *)data_jalloc.allocate();
            new (res->val) T(v);
            return res;
        }
        void return_node(void *lp)
        {
            jalloc.deallocate(lp);
        }
        node *root;

        int max(int a, int b)
        {
            if (a > b)
                return a;
            return b;
        }
        // void update_height(node *&lp);
        void update_height(node *&lp)
        {
            if (lp->lc)
            {
                lp->lh = max(lp->lc->lh, lp->lc->rh) + 1;
            }
            else
            {
                lp->lh = 0;
            }

            if (lp->rc)
            {
                lp->rh = max(lp->rc->lh, lp->rc->rh) + 1;
            }
            else
            {
                lp->rh = 0;
            }
        }
        void lblc(node *&nd)
        {
            node *rch = nd->rc;
            nd->rc = rch->lc;
            rch->lc = nd;
            nd = rch;
            update_height(nd->lc);
            update_height(nd);
        }
        void rblc(node *&nd)
        {
            node *lch = nd->lc;
            nd->lc = lch->rc;
            lch->rc = nd;
            nd = lch;
            update_height(nd->rc);
            update_height(nd);
        }
        void test_fix(node *&lp)
        {
            node *lch = lp->lc;
            node *rch = lp->rc;
            if (2 == lp->lh - lp->rh)
            {
                if (lch->lh > lch->rh)
                {
                    rblc(lp);
                }
                else
                {
                    lblc(lp->lc);

                    rblc(lp);
                }
            }
            else if (-2 == lp->lh - lp->rh)
            {
                if (rch->rh > rch->lh)
                {
                    lblc(lp);
                }
                else
                {
                    rblc(lp->rc);

                    lblc(lp);
                }
            }
        }
        int insert(long key, T val, node *&lp)
        {
            if (lp)
            {
                if (lp->key > key)
                {
                    lp->lh = insert(key, val, lp->lc);
                }
                else if (lp->key < key)
                {
                    lp->rh = insert(key, val, lp->rc);
                }
                else
                {
                    return max(lp->lh, lp->rh) + 1;
                }
                test_fix(lp);
            }
            else
            {
                lp = create_node(key, val);
            }
            return max(lp->lh, lp->rh) + 1;
        }
        int del(long key, node *&lp)
        {
            if (lp)
            {
                if (lp->key > key)
                {
                    lp->lh = del(key, lp->lc);
                    test_fix(lp);
                }
                else if (lp->key < key)
                {
                    lp->rh = del(key, lp->rc);
                    test_fix(lp);
                }
                else
                {
                    if (!lp->lc && !lp->rc)
                    {
                        node *q = lp;
                        lp = nullptr;
                        return_node(q);
                        return 0;
                    }
                    else if (!lp->rc)
                    {
                        node *q = lp;
                        lp = lp->lc;
                        return_node(q);
                    }
                    else if (!lp->lc)
                    {
                        node *q = lp;
                        lp = lp->rc;
                        return_node(q);
                    }
                    else
                    {
                        // std::cerr<<lp->key;
                        node *q = lp->lc;
                        while (q->rc)
                        {
                            q = q->rc;
                        }
                        lp->key = q->key;
                        lp->val = q->val;
                        lp->lh = del(q->key, lp->lc);
                        test_fix(lp);
                    }
                }
            }
            else
            {
                return 0;
            }
            return max(lp->lh, lp->rh) + 1;
        }
        node *find(long key, node *&lp)
        {
            if (lp)
            {
                if (lp->key > key)
                {
                    return find(key, lp->lc);
                }
                else if (lp->key < key)
                {
                    return find(key, lp->rc);
                }
            }
            return lp;
        }
        T *try_find(long key)
        {
            node *res;
            res = find(key, root);
            if (!res)
            {
                return nullptr;
            }
            T *lp = res->val;
            return lp;
        }

    public:
        T *find(long key)
        {
            pthread_rwlock_rdlock(&prw);
            T *res = try_find(key);
            pthread_rwlock_unlock(&prw);
            return res;
        }
        bool insert(long key, T val)
        {
            pthread_rwlock_wrlock(&prw);
            insert(key, val, root);
            pthread_rwlock_unlock(&prw);
            return true;
        }
        bool del(long key)
        {
            pthread_rwlock_wrlock(&prw);
            del(key, root);
            pthread_rwlock_unlock(&prw);
            return true;
        }
        void update(long key, T v)
        {
            pthread_rwlock_wrlock(&prw);
            node *res = find(key, root);
            if (res)
            {

                *(res->val) = v;

            }
            pthread_rwlock_unlock(&prw);
        }
        void return_data(void *lp)
        {
            data_jalloc.deallocate(lp);
        }
        bool is_empty()
        {
            return (root == nullptr);
        }
        long get_top()
        {
            return root->key;
        }
        jtree_avl();
        ~jtree_avl();
    };

    template <typename T, template <typename N> class jalloc_t>
    jtree_avl<T, jalloc_t>::jtree_avl(/* args */) : jalloc(), root(nullptr)
    {
        pthread_rwlock_init(&prw,0);
    }

    template <typename T, template <typename N> class jalloc_t>
    jtree_avl<T, jalloc_t>::~jtree_avl()
    {
    }

/*




















 */

#define red 0
#define black 1
    /* thread safe redblack tree */
    template <typename T, template <typename N> class jalloc_t = jalloc_conc>
    class jtree_rb // not thread-safe
    {
    private:
        T operator=(jtree_rb &) {}
        jtree_rb(jtree_rb &) {}
        jalloc_t<T> data_jalloc;
        struct node
        {
            /* data */
            node *pa;
            node *lc, *rc;
            bool color;
            long key;
            T *val;
            node(long k) : pa(nullptr), lc(nullptr), rc(nullptr), color(red), key(k), val()
            {
            }
        };
        node *root;
        node *test_root;
        jalloc_t<node> jalloc;
        pthread_rwlock_t prw;
        node *create_node(long k, T &v)
        {
            // node *lp = new node(k, v); // delete
            node *res = (node *)jalloc.allocate();
            new (res) node(k);
            res->val = (T *)data_jalloc.allocate();
            new (res->val) T(v);
            return res;
        }
        void return_node(void *lp)
        {
            jalloc.deallocate(lp);
        }
        void copy_b2a(node *&a, node *&b)
        {
            a->key = b->key;
            a->val = b->val;
        }

        void lblc(node *&nd)
        {
            node *rc = nd->rc;
            node *rclc = rc->lc;
            rc->pa = nd->pa;
            if (rclc)
                rclc->pa = nd;
            nd->pa = rc;

            nd->rc = rclc;
            rc->lc = nd;
            nd = rc;
        }
        void rblc(node *&nd)
        { // cout
            node *lc = nd->lc;
            node *lcrc = lc->rc;
            lc->pa = nd->pa;
            if (lcrc)
                lcrc->pa = nd;
            nd->pa = lc;

            nd->lc = lcrc;
            lc->rc = nd;
            nd = lc;
        }
        void test_fix(node *&);
        void insert(long k, T v, node *&nd);
        T *find(long, node *&);
        void deal_black_leaf(node *&, bool);
        void del(long, node *&);
        void recurse_fix(node *&);
        void dcr_black_height(node *&);

    public:
        jtree_rb();
        T *find(long k)
        {
            pthread_rwlock_rdlock(&prw);
            T *r = find(k, root);
            pthread_rwlock_unlock(&prw);
            return r;
        }
        void insert(long k, T v)
        {
            pthread_rwlock_wrlock(&prw);
            if (!root)
            {
                root = create_node(k, v);
                root->color = black;
                return;
            }
            
            insert(k, v, root);
            root->color = black;
            pthread_rwlock_unlock(&prw);
        }
        void del(long k)
        {
            if(!find(k)){
                return;
            }
            pthread_rwlock_wrlock(&prw);
            del(k, root);
            pthread_rwlock_unlock(&prw);
        }
        void update(long k,T&v){
            T*p=nullptr;
            if(!(p=find(k))){
                return;
            }
            *p=v;
        }
        void return_data(void *lp)
        {
            data_jalloc.deallocate(lp);
        }
    };
    template <typename T, template <typename N> class jalloc_t>
    void jtree_rb<T, jalloc_t>::test_fix(node *&nd)
    {
        if (!nd->pa || !nd->pa->pa)
        {
            test_root = nullptr;
            return;
        }
        if ((nd->color == red) && (nd->pa->color == red))
        {
            node *pa = nd->pa;
            node *gpa = pa->pa;
            if (gpa->lc == pa)
            {
                node *cou = gpa->rc;
                if (!cou || cou->color == black)
                {
                    if (nd == pa->rc)
                    {
                        test_root = nd;
                        nd->color = black;
                        gpa->color = red;
                        lblc(gpa->lc);
                    }
                    else
                    {
                        test_root = pa;
                        pa->color = black;
                        gpa->color = red;
                    }

                    if (gpa == root)
                    {
                        rblc(root);
                    }
                    else
                    {
                        if (gpa == gpa->pa->lc)
                            rblc(gpa->pa->lc);
                        else
                            rblc(gpa->pa->rc);
                    }
                }
                else
                {
                    test_root = gpa;
                    cou->color = black;
                    pa->color = black;
                    gpa->color = red;
                }
            }
            else
            {
                node *cou = gpa->lc;
                if (!cou || cou->color == black)
                {
                    if (nd == pa->lc)
                    {
                        test_root = nd;
                        nd->color = black;
                        gpa->color = red;
                        rblc(gpa->rc);
                    }
                    else
                    {
                        test_root = pa;
                        pa->color = black;
                        gpa->color = red;
                    }

                    if (gpa == root)
                    {
                        lblc(root);
                    }
                    else
                    {
                        if (gpa == gpa->pa->rc)
                            lblc(gpa->pa->rc);
                        else
                            lblc(gpa->pa->lc);
                    }
                }
                else
                {
                    test_root = gpa;
                    cou->color = black;
                    pa->color = black;
                    gpa->color = red;
                }
            }
        }
        else
        {
            test_root = nullptr;
        }
    }

    template <typename T, template <typename N> class jalloc_t>
    jtree_rb<T, jalloc_t>::jtree_rb() : root(nullptr), test_root(nullptr), jalloc() {
        pthread_rwlock_init(&prw,0);
    }

    template <typename T, template <typename N> class jalloc_t>
    T *jtree_rb<T, jalloc_t>::find(long k, node *&nd)
    {
        if (!nd)
            return nullptr;
        if (nd->key > k)
        {
            return find(k, nd->lc);
        }
        else if (nd->key < k)
        {
            return find(k, nd->rc);
        }
        T *lp = nd->val;
        return lp;
    }

    template <typename T, template <typename N> class jalloc_t>
    void jtree_rb<T, jalloc_t>::insert(long k, T v, node *&nd)
    {
        if (nd)
        {
            if (nd->key > k)
            {
                if (nd->lc)
                    insert(k, v, nd->lc);
                else
                {
                    nd->lc = create_node(k, v);
                    nd->lc->pa = nd;
                    test_fix(nd->lc);
                }
            }
            else if (nd->key < k)
            {
                if (nd->rc)
                    insert(k, v, nd->rc);
                else
                {
                    nd->rc = create_node(k, v);
                    nd->rc->pa = nd;
                    test_fix(nd->rc);
                }
            }
            else
            {
                return;
            }
        }
        while (test_root)
        {
            if (test_root == root)
            {
                test_fix(root);
            }
            else
            {
                if (test_root->pa->lc == test_root)
                {
                    test_fix(test_root->pa->lc);
                }
                else
                {
                    test_fix(test_root->pa->rc);
                }
            }
        }
    }
    template <typename T, template <typename N> class jalloc_t>
    void jtree_rb<T, jalloc_t>::del(long k, node *&nd)
    {
        if (!nd)
            return;
        if (nd->key > k)
        {
            del(k, nd->lc);
        }
        else if (nd->key < k)
        {
            del(k, nd->rc);
        }
        else
        {
            if (nd->lc && nd->rc)
            { // has two child
                node *b = nd->lc;
                while (b->rc)
                    b = b->rc;
                copy_b2a(nd, b);
                del(b->key, nd->lc);
            }
            else
            {
                if (!(nd->lc || nd->rc))
                {                         // the leaf node
                    if (nd->color == red) // red leaf
                    {
                        node *q = nd;
                        nd = nullptr;
                        // free(q);
                        return_node(q);
                        return;
                    }
                    else
                    { // black leaf
                        recurse_fix(nd);
                        node *q = nd;
                        nd = nullptr;
                        return_node(q);
                        return;
                    }
                }
                else
                { // has one child,and child must be red
                    if (nd->lc)
                    {
                        copy_b2a(nd, nd->lc);
                        del(nd->lc->key, nd->lc);
                    }
                    else
                    {
                        copy_b2a(nd, nd->rc);
                        del(nd->rc->key, nd->rc);
                    }
                }
            }
        }
    }
    template <typename T, template <typename N> class jalloc_t> // delete
    void jtree_rb<T, jalloc_t>::recurse_fix(node *&nd)          // a closure
    {
        if (nd == root)
        {
            return;
        }
        if (nd->pa->color == red)
        { // red pa,black nd(lc),must there be a black rc
            if (nd == nd->pa->lc)
            { // nd is lc
                if (nd->pa->rc->rc && nd->pa->rc->rc->color == red)
                {
                    nd->pa->rc->rc->color = black;
                    nd->pa->color = black;
                    nd->pa->rc->color = red;
                    if (nd->pa == root)
                    {
                        lblc(root);
                    }
                    else if (nd->pa->pa->lc == nd->pa)
                    {
                        lblc(nd->pa->pa->lc);
                    }
                    else
                    {
                        lblc(nd->pa->pa->rc);
                    }
                }
                else if (nd->pa->rc->lc && nd->pa->rc->lc->color == red)
                {
                    nd->pa->rc->color = red;
                    nd->pa->rc->lc->color = black;
                    rblc(nd->pa->rc);
                    if (nd->pa == root)
                    {
                        lblc(root);
                    }
                    else if (nd->pa->pa->lc == nd->pa)
                    {
                        lblc(nd->pa->pa->lc);
                    }
                    else
                    {
                        lblc(nd->pa->pa->rc);
                    }
                }
                else
                { // nd-pa-rc has no red child
                    if (nd->pa == root)
                    {
                        lblc(root);
                    }
                    else if (nd->pa->pa->lc == nd->pa)
                    {
                        lblc(nd->pa->pa->lc);
                    }
                    else
                    {
                        lblc(nd->pa->pa->rc);
                    }
                }
            }
            else // nd is rc
            {
                if (nd->pa->lc->lc && nd->pa->lc->lc->color == red)
                {
                    nd->pa->lc->lc->color = black;
                    nd->pa->color = black;
                    nd->pa->lc->color = red;
                    if (nd->pa == root)
                    {
                        rblc(root);
                    }
                    else if (nd->pa->pa->rc == nd->pa)
                    {
                        rblc(nd->pa->pa->rc);
                    }
                    else
                    {
                        rblc(nd->pa->pa->lc);
                    }
                }
                else if (nd->pa->lc->rc && nd->pa->lc->rc->color == red)
                {
                    nd->pa->lc->color = red;
                    nd->pa->lc->rc->color = black;
                    lblc(nd->pa->lc);
                    if (nd->pa == root)
                    {
                        rblc(root);
                    }
                    else if (nd->pa->pa->rc == nd->pa)
                    {
                        rblc(nd->pa->pa->rc);
                    }
                    else
                    {
                        rblc(nd->pa->pa->lc);
                    }
                }
                else
                { // red pa -> black cousin has no red child
                    if (nd->pa == root)
                    {
                        rblc(root);
                    }
                    else if (nd->pa->pa->lc == nd->pa)
                    {
                        rblc(nd->pa->pa->lc);
                    }
                    else
                    {
                        rblc(nd->pa->pa->rc);
                    }
                }
            }
        }
        else
        {
            // black parent
            if (nd == nd->pa->lc)
            {                                   // nd is lc
                if (nd->pa->rc->color == black) // one black leaf and a black parent will made parent a black leaf equal
                {                               // time for recursion,black height --
                    // nd->pa->rc->color = red;
                    // recurse_fix(nd->pa->c);
                    if (nd->pa->rc->rc && nd->pa->rc->rc->color == red)
                    {
                        nd->pa->rc->rc->color = black;
                        if (nd->pa == root)
                        {
                            lblc(root);
                        }
                        else if (nd->pa->pa->rc == nd->pa)
                        {
                            lblc(nd->pa->pa->rc);
                        }
                        else
                        {
                            lblc(nd->pa->pa->lc);
                        }
                    }
                    else if (nd->pa->rc->lc && nd->pa->rc->lc->color == red)
                    {
                        nd->pa->rc->lc->color = black;
                        rblc(nd->pa->rc);
                        if (nd->pa == root)
                        {
                            lblc(root);
                        }
                        else if (nd->pa->pa->rc == nd->pa)
                        {
                            lblc(nd->pa->pa->rc);
                        }
                        else
                        {
                            lblc(nd->pa->pa->lc);
                        }
                    }
                    else
                    { // black only three ,recurse cout
                        nd->pa->rc->color = red;
                        if (nd->pa == root)
                        {
                            return;
                        }
                        else
                        {
                            if (nd->pa->pa->lc == nd->pa)
                            {
                                recurse_fix(nd->pa->pa->lc);
                            }
                            else
                            {
                                recurse_fix(nd->pa->pa->rc);
                            }
                        }
                    }
                }
                else
                { // black parent, red cousin
                    nd->pa->rc->color = black;
                    if (nd->pa->rc->lc->lc && nd->pa->rc->lc->rc && nd->pa->rc->lc->lc->color == red && nd->pa->rc->lc->rc->color == red)
                    {
                        nd->pa->rc->lc->color = red;
                        nd->pa->rc->lc->rc->color = black;
                        if (nd->pa == root)
                        {
                            lblc(root);
                            lblc(root->lc);
                        }
                        else if (nd->pa->pa->lc == nd->pa)
                        {
                            node *gpa = nd->pa->pa;
                            lblc(gpa->lc);
                            lblc(gpa->lc->lc);
                        }
                        else
                        {
                            node *gpa = nd->pa->pa;
                            lblc(gpa->rc);
                            lblc(gpa->rc->lc);
                        }
                    }
                    else if (nd->pa->rc->lc->lc && nd->pa->rc->lc->lc->color == red)
                    {
                        if (nd->pa == root)
                        {
                            lblc(root);
                            rblc(root->lc->rc);
                            lblc(root->lc);
                        }
                        else if (nd->pa->pa->lc == nd->pa)
                        {
                            node *gpa = nd->pa->pa;
                            lblc(gpa->lc);
                            rblc(gpa->lc->lc->rc);
                            lblc(gpa->lc->lc);
                        }
                        else
                        {
                            node *gpa = nd->pa->pa;
                            lblc(gpa->rc);
                            rblc(gpa->rc->lc->rc);
                            lblc(gpa->rc->lc);
                        }
                    }
                    else if (nd->pa->rc->lc->rc && nd->pa->rc->lc->rc->color == red)
                    {
                        nd->pa->rc->lc->color = red;
                        nd->pa->rc->lc->rc->color = black;
                        if (nd->pa == root)
                        {
                            lblc(root);
                            lblc(root->lc);
                        }
                        else if (nd->pa->pa->lc == nd->pa)
                        {
                            node *gpa = nd->pa->pa;
                            lblc(gpa->lc);
                            lblc(gpa->lc->lc);
                        }
                        else
                        {
                            node *gpa = nd->pa->pa;
                            lblc(gpa->rc);
                            lblc(gpa->rc->lc);
                        }
                    }
                    else
                    { // cousin has no red GRAND child
                        nd->pa->rc->color = black;
                        if (nd->pa->rc->lc)
                        { // cou has a black
                            nd->pa->rc->lc->color = red;
                            if (nd->pa == root)
                            {
                                lblc(root);
                            }
                            else if (nd->pa == nd->pa->pa->lc)
                            {
                                lblc(nd->pa->pa->lc);
                            }
                            else
                            {
                                lblc(nd->pa->pa->rc);
                            }
                        }
                    }
                }
            }
            else
            { // nd is rc
                if (nd->pa->lc->color == black)
                {
                    if (nd->pa->lc->lc && nd->pa->lc->lc->color == red)
                    {
                        nd->pa->lc->lc->color = black;
                        if (nd->pa == root)
                        {
                            rblc(root);
                        }
                        else if (nd->pa->pa->rc == nd->pa)
                        {
                            rblc(nd->pa->pa->rc);
                        }
                        else
                        {
                            rblc(nd->pa->pa->lc);
                        }
                    }
                    else if (nd->pa->lc->rc && nd->pa->lc->rc->color == red)
                    {
                        nd->pa->lc->rc->color = black;
                        lblc(nd->pa->lc);
                        if (nd->pa == root)
                        {
                            rblc(root);
                        }
                        else if (nd->pa->pa->rc == nd->pa)
                        {
                            rblc(nd->pa->pa->rc);
                        }
                        else
                        {
                            rblc(nd->pa->pa->lc);
                        }
                    }
                    else
                    { // three only black, recurse
                        nd->pa->lc->color = red;
                        if (nd->pa == root)
                        {
                            return;
                        }
                        else
                        {
                            if (nd->pa->pa->lc == nd->pa)
                            {
                                recurse_fix(nd->pa->pa->lc);
                            }
                            else
                            {
                                recurse_fix(nd->pa->pa->rc);
                            }
                        }
                    }
                }
                else // black pa, red cousin
                {
                    nd->pa->lc->color = black;
                    if (nd->pa->lc->rc->rc && nd->pa->lc->rc->lc && nd->pa->lc->rc->rc->color == red && nd->pa->lc->rc->lc->color == red)
                    {
                        nd->pa->lc->rc->color = red;
                        nd->pa->lc->rc->lc->color = black;
                        if (nd->pa == root)
                        {
                            rblc(root);
                            rblc(root->rc);
                        }
                        else if (nd->pa->pa->rc == nd->pa)
                        {
                            node *gpa = nd->pa->pa;
                            rblc(gpa->rc);
                            rblc(gpa->rc->rc);
                        }
                        else
                        {
                            node *gpa = nd->pa->pa;
                            rblc(gpa->lc);
                            rblc(gpa->lc->rc);
                        }
                    }
                    else if (nd->pa->lc->rc->rc && nd->pa->lc->rc->rc->color == red)
                    {
                        if (nd->pa == root)
                        {
                            rblc(root);
                            lblc(root->rc->lc);
                            rblc(root->rc);
                        }
                        else if (nd->pa->pa->rc == nd->pa)
                        {
                            node *gpa = nd->pa->pa;
                            rblc(gpa->rc);
                            lblc(gpa->rc->rc->lc);
                            rblc(gpa->rc->rc);
                        }
                        else
                        {
                            node *gpa = nd->pa->pa;
                            rblc(gpa->lc);
                            lblc(gpa->lc->rc->lc);
                            rblc(gpa->lc->rc);
                        }
                    }
                    else if (nd->pa->lc->rc->lc && nd->pa->lc->rc->lc->color == red)
                    {
                        // std::cout<<nd->rc->rc->rc->rc->rc->key<<std::endl;
                        // test_proper(nd->rc->rc->rc->rc->rc);
                        nd->pa->lc->rc->color = red;
                        nd->pa->lc->rc->lc->color = black;
                        if (nd->pa == root)
                        {
                            rblc(root);
                            rblc(root->rc);
                        }
                        else if (nd->pa->pa->rc == nd->pa)
                        {
                            node *gpa = nd->pa->pa;
                            rblc(gpa->rc);
                            rblc(gpa->rc->rc);
                        }
                        else
                        {
                            node *gpa = nd->pa->pa;
                            rblc(gpa->lc);
                            rblc(gpa->lc->rc);
                        }
                    }
                    else
                    {
                        if (nd->pa->lc->rc)
                        { // cou has a black
                            nd->pa->lc->rc->color = red;
                            if (nd->pa == root)
                            {
                                rblc(root);
                            }
                            else if (nd->pa == nd->pa->pa->rc)
                            {
                                rblc(nd->pa->pa->rc);
                            }
                            else
                            {
                                rblc(nd->pa->pa->lc);
                            }
                        }
                    }
                }
            }
        }
    }
}
#undef mfence
#endif