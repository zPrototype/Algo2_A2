// Vorzeichenlose ganze Zahl.
using uint = unsigned int;
using namespace std;

// Als Binomial-Halde implementierte Minimum-Vorrangwarteschlange
// mit Prioritäten des Typs P und zusätzlichen Daten des Typs D.
// An der Stelle, an der BinHeap für einen bestimmten Typ P verwendet
// wird, muss ein Kleiner-Operator (<) für den Typ P bekannt sein.
// Werte des Typs P dürfen nur mit diesem Operator verglichen werden;
// die übrigen Vergleichsoperatoren (<=, >, >=, ==, !=) dürfen nicht
// verwendet werden.
template<typename P, typename D>
struct BinHeap
{
    // Notwendige Vorabdeklaration.
    struct Node;

    // Eintrag einer solchen Warteschlange bzw. Halde, bestehend aus
    // einer Priorität prio mit Typ P und zusätzlichen Daten data mit
    // Typ D.
    // Wenn der Eintrag momentan tatsächlich zu einer Halde gehört,
    // verweist node auf den zugehörigen Knoten eines Binomialbaums
    // dieser Halde; andernfalls ist node ein Nullzeiger.
    struct Entry
    {
        // Priorität, zusätzliche Daten und zugehöriger Knoten.
        P prio;
        D data;
        Node *node;

        // Initialisierung mit Priorität p und zusätzlichen Daten d.
        Entry(P p, D d) : prio(p), data(d), node(nullptr)
        {}
    };

    // Knoten eines Binomialbaums innerhalb einer solchen Halde.
    // Neben den eigentlichen Knotendaten (degree, parent, child,
    // sibling), enthält der Knoten einen Verweis auf den zugehörigen
    // Eintrag.
    struct Node
    {
        // Zugehöriger Eintrag.
        Entry *entry;

        // Grad des Knotens.
        uint degree;

        // Vorgänger (falls vorhanden; Nullzeiger bei einem Wurzelknoten).
        Node *parent;

        // Nachfolger mit dem größten Grad
        // (falls vorhanden; Nullzeiger bei einem Blattknoten).
        Node *child;

        // Zirkuläre Verkettung aller Nachfolger eines Knotens
        // bzw. einfache Verkettung aller Wurzelknoten einer Halde,
        // jeweils sortiert nach aufsteigendem Grad.
        Node *sibling;

        // Initialisierung als Wurzelknoten mit Grad 0 und Eintrag e,
        // der dann auf diesen Knoten zurückverweist.
        Node(Entry *e) : entry(e), degree(0),
                         parent(nullptr), child(nullptr), sibling(nullptr)
        {
            e->node = this;
        }

        bool operator<(Node *const &other) const
        {
            return entry->prio < other->entry->prio;
        }
    };

    // Anfang der Wurzelliste (Nullzeiger bei einer leeren Halde).
    Node *head;
    uint heapSize;

    // Initialisierung als leere Halde.
    BinHeap<P, D>()
    {
        heapSize = 0;
        head = nullptr;
    }

    // Größe der Halde, d. h. Anzahl momentan gespeicherter Einträge
    // liefern.
    uint size()
    {
        return heapSize;
    }

    // Neuen Eintrag mit Priorität p und zusätzlichen Daten d erzeugen,
    // zur Halde hinzufügen und zurückliefern.
    Entry *insert(P p, D d)
    {
        Entry *e = new Entry(p, d);
        Node *n = new Node(e);
        n->degree = 0;

        BinHeap<P, D> *insHeap = new BinHeap<P, D>();
        insHeap->head = n;
        insHeap->heapSize = 1;
        this->heapSize = this->heapSize + insHeap->heapSize;
        this->head = mergeHeap(this, insHeap);
        return e;
    }

    // Eintrag mit minimaler Priorität liefern.
    // (Nullzeiger bei einer leeren Halde.)
    Entry *minimum()
    {
        if (head == 0)
        {
            return nullptr;
        } else
        {
            Node *min = head;
            Node *next = min->sibling;
            while (next != 0)
            {
                if (next->operator<(min))
                {
                    min = next;
                }
                next = next->sibling;
            }
            Entry *result = min->entry;
            return result;
        }
    }

    // Eintrag mit minimaler Priorität liefern
    // und aus der Halde entfernen (aber nicht freigeben).
    // (Bei einer leeren Halde wirkungslos mit Nullzeiger als Resultatwert.)
    Entry *extractMin()
    {
        if (head == 0)
        {
            return nullptr;
        }

        Node *min = head;
        Node *minPrev = nullptr;
        Node *next = min->sibling;
        Node *nextPrev = min;

        while (next != 0)
        {
            if (next->operator<(min))
            {
                min = next;
                minPrev = nextPrev;
            }
            nextPrev = next;
            next = next->sibling;
        }
        removeTreeRoot(min, minPrev);
        this->heapSize -= 1;
        return min->entry;
    }

    // Enthält die Halde den Eintrag e?
    // Resultatwert false, wenn e ein Nullzeiger ist.
    bool contains(Entry *e)
    {
        if (e != 0)
        {
            // falls der eintrag in der Halde existiert, so muss der Knoten zum head führen
            Node *findNode = e->node;
            if (findNode != 0)
            {
                if (e == findNode->entry)
                {
                    // so lange hoch bis kein parent existiert
                    while (findNode->parent != 0)
                    {
                        findNode = findNode->parent;
                    }

                    // wenn einer der siblings der head ist -> eintrag e existiert in der Halde
                    for (Node *h = head; h != 0; h = h->sibling)
                    {
                        if (h->entry == findNode->entry)
                        {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    // Priorität des Eintrags e auf p ändern.
    // Hierbei darf auf keinen Fall ein neues Entry-Objekt entstehen,
    // selbst wenn die Operation intern als Entfernen und Neu-Einfügen
    // implementiert wird.
    // (Wirkungslos mit Resultatwert false, wenn e ein Nullzeiger ist
    // oder e nicht zur aktuellen Halde gehört.)
    bool changePrio(Entry *e, P p)
    {
        if (this->contains(e) && e != 0 && e->prio != p)
        {
            if (e->prio > p)
            {
                Node *node = e->node;
                node = bubbleUp(node, false);
            } else
            {
                Node *node = e->node;
                node = moveDown(node, false);
            }
            return true;
        }
        return false;
    }

    // Eintrag e aus der Halde entfernen (aber nicht freigeben).
    // (Wirkungslos mit Resultatwert false, wenn e ein Nullzeiger ist
    // oder e nicht zur aktuellen Halde gehört.)
    bool remove(Entry *e)
    {
        if (e == 0 || !this->contains(e))
        {
            return false;
        }
        Node *node = e->node;
        node = bubbleUp(node, true);
        if (head == node)
        {
            removeTreeRoot(node, nullptr);
        } else
        {
            Node *prev = this->head;
            while (prev->sibling->operator<(node) || node->operator<(prev->sibling))
            {
                prev = prev->sibling;
            }
            removeTreeRoot(node, prev);
        }
        this->heapSize -= 1;
        return true;
    }

    // Inhalt der Halde zu Testzwecken ausgeben.
    void dump()
    {
        cout << endl;
        for (Node *node = this->head; node != 0; node = node->sibling)
        {
            dump(node, 0);
        }
    }

    void dump(Node *node, uint pos)
    {
        if (node != 0)
        {
            for (uint i = 0; i < pos; i++)
            {
                cout << " ";
            }
            cout << node->entry->prio << " " << node->entry->data << endl;
            if (node->child != 0)
            {
                if (node->child->sibling != 0)
                {
                    dumpSibling(node->child->sibling, pos + 1);
                } else
                {
                    dump(node->child, pos + 1);
                }
            }
        }
    }

    void dumpSibling(Node *start, uint pos)
    {
        Node *nextNode = start;
        do
        {
            for (int i = 0; i < pos; i++)
            {
                cout << " ";
            }

            cout << nextNode->entry->prio << " " << nextNode->entry->data << endl;

            if (nextNode->child != 0)
            {
                if (nextNode->child->sibling != 0)
                {
                    dumpSibling(nextNode->child->sibling, pos + 1);
                } else
                {
                    dump(nextNode->child, pos + 1);
                }
            }
            nextNode = nextNode->sibling;
        } while (start != nextNode && nextNode != 0);
    }

    Node *mergeHeap(BinHeap<P, D> *h1, BinHeap<P, D> *h2)
    {
        BinHeap<P, D> *h3 = new BinHeap<P, D>();
        if (h1->head == 0 && h2->head == 0)
        {
            return nullptr;
        }
        if (h1->head == 0)
        {
            return h2->head;
        }
        if (h2->head == 0)
        {
            return h1->head;
        }

        Node *n1 = nullptr;
        Node *n2 = nullptr;
        Node *n3 = nullptr;

        uint d = 0; // d ist der Grad

        while (h1->head != 0 || h2->head != 0 || n1 != 0 || n2 != 0 || n3 != 0)
        {
            if (h1->head != 0)
            {
                if (h1->head->degree == d)//Schritt 1
                {
                    n1 = h1->head;
                    h1->head = h1->head->sibling;
                }
            }

            if (h2->head != 0)
            {
                if (h2->head->degree == d)//Schritt 2
                {
                    n2 = h2->head;
                    h2->head = h2->head->sibling;
                }
            }

            //Schritt 3
            if (n1 != 0 && n2 == 0 && n3 == 0)
            {
                if (h3->head == 0)
                {
                    h3->head = n1;
                } else
                {
                    Node *last = h3->head;
                    Node *temp = n1;

                    while (last->sibling != 0)
                    {
                        last = last->sibling;
                    }
                    last->sibling = temp;
                    temp->sibling = nullptr;
                }
                n1 = nullptr;
            } else if (n1 == 0 && n2 != 0 && n3 == 0)
            {
                if (h3->head == 0)
                {
                    h3->head = n2;
                } else
                {
                    Node *last = h3->head;
                    Node *temp = n2;

                    while (last->sibling != 0)
                    {
                        last = last->sibling;
                    }
                    last->sibling = temp;
                    temp->sibling = nullptr;
                }
                n2 = nullptr;
            } else if (n1 == 0 && n2 == 0 && n3 != 0)
            {
                if (h3->head == 0)
                {
                    h3->head = n3;
                } else
                {
                    Node *last = h3->head;
                    Node *temp = n3;

                    while (last->sibling != 0)
                    {
                        last = last->sibling;
                    }
                    last->sibling = temp;
                    temp->sibling = nullptr;
                }
                n3 = nullptr;
            } else if ((n1 != 0 && n2 != 0) && n3 != 0)
            {
                if (h3->head == 0)
                {
                    h3->head = n3;
                } else
                {
                    Node *last = h3->head;
                    Node *temp = n3;

                    while (last->sibling != 0)
                    {
                        last = last->sibling;
                    }
                    last->sibling = temp;
                    temp->sibling = nullptr;
                }
                n3 = nullptr;
            }

            //Schritt 4

            Node *tmpTree1 = nullptr;
            Node *tmpTree2 = nullptr;

            if (n1 != 0 && n2 != 0)
            {
                tmpTree1 = n1;
                tmpTree2 = n2;
            } else if (n1 != 0 && n3 != 0)
            {
                tmpTree1 = n1;
                tmpTree2 = n3;
            } else if (n2 != 0 && n3 != 0)
            {
                tmpTree1 = n2;
                tmpTree2 = n3;
            }

            n1 = nullptr;
            n2 = nullptr;
            n3 = nullptr;

            if (tmpTree1 != 0 && tmpTree2 != 0)
            {
                if (tmpTree2->operator<(tmpTree1))
                {
                    tmpTree2->sibling = nullptr;
                    tmpTree2->degree = tmpTree2->degree + 1;

                    tmpTree1->parent = tmpTree2;
                    if (tmpTree2->child == 0)
                    {
                        tmpTree2->child = tmpTree1->sibling = tmpTree1;
                    } else
                    {
                        tmpTree1->sibling = tmpTree2->child->sibling;
                        tmpTree2->child = tmpTree2->child->sibling = tmpTree1;
                    }
                    n3 = tmpTree2;

                } else
                {
                    tmpTree1->sibling = nullptr;
                    tmpTree1->degree = tmpTree1->degree + 1;

                    tmpTree2->parent = tmpTree1;
                    if (tmpTree1->child == 0)
                    {
                        tmpTree1->child = tmpTree2->sibling = tmpTree2;
                    } else
                    {
                        tmpTree2->sibling = tmpTree1->child->sibling;
                        tmpTree1->child = tmpTree1->child->sibling = tmpTree2;
                    }
                    n3 = tmpTree1;
                }
            }
            d++;
        }
        return h3->head;
    }

    Node *bubbleUp(Node *node, bool toRoot)
    {
        Node *parent = node->parent;
        while (parent != 0 && (toRoot || node->operator<(parent)))
        {
            Entry *temp = node->entry;
            node->entry = parent->entry;
            parent->entry = temp;

            node = parent;
            parent = parent->parent;
        }
        return node;
    }

    Node *moveDown(Node *node, bool toRoot)
    {
        Node *child = node->child;
        while (child != 0 && (toRoot || child->operator<(node)))
        {
            Entry *temp = node->entry;
            node->entry = child->entry;
            child->entry = temp;

            node->child = child;
            child = child->child;
        }
        return node;
    }

    void removeTreeRoot(Node *root, Node *prev)
    {
        // Remove root from the heap
        if (root == head)
        {
            head = root->sibling;
        } else
        {
            prev->sibling = root->sibling;
        }

        // Reverse the order of root's children and make a new heap
        Node *newHead = nullptr;
        Node *child = root->child;
        while (child != 0)
        {
            Node *next = child->sibling;
            child->sibling = newHead;
            child->parent = nullptr;
            newHead = child;
            child = next;
        }

        BinHeap<P, D> *newHeap = new BinHeap<P, D>();
        newHeap->head = newHead;
        newHeap->heapSize = 1;
        this->head = mergeHeap(this, newHeap);
    }
};

