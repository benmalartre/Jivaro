#ifndef JVR_GRAPH_GRAPH_H
#define JVR_GRAPH_GRAPH_H

#include <pxr/pxr.h>
#include "pxr/base/tf/type.h"
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE

static GfVec2f DEFAULT_NODE_SIZE(120.f, 60.f);
static GfVec3f DEFAULT_NODE_COLOR(0.5f, 0.5f, 0.5f);

static TfToken ParentPortToken("Parent");
static TfToken ChildrenPortToken("Children");

class Graph 
{
public:

  enum Type {
    PIPELINE,
    HIERARCHY,
    MATERIAL,
    EXECUTION
  };
 
  class Node;
  class Port;
  class Connexion;

  // Graph port class
  //-------------------------------------------------------------------
  class Port {
    public:

      enum Flag {
        INPUT = 1,
        OUTPUT = 2,
        INTERNAL = 4,
        HIDDEN = 8,
        HORIZONTAL = 16,
        VERTICAL = 32
      };

      Port() {};
      Port(Node* node, size_t flag, const TfToken& label, 
        UsdAttribute& attribute);
      Port(Node* node, size_t flag, const TfToken& label);

      bool IsInput() { return _flags & INPUT; };
      bool IsOutput() { return _flags & OUTPUT; };
      bool IsBothInputOutput() { return _flags & (INPUT | OUTPUT); };

      const TfToken& GetName()const {return _label;};
      SdfPath GetPath();
      const Node* GetNode() const { return _node; };
      Node* GetNode() { return _node; };
      void SetNode(Node* node) { _node = node; };
      const UsdAttribute& GetAttr() const { return _attr;};
      UsdAttribute& GetAttr() { return _attr;};
      size_t GetFlags() { return _flags; };
      TfToken GetLabel() { return _label; };
      bool IsConnected(Graph* graph, Connexion* foundConnexion);

    protected:
      Node*                 _node;
      TfToken               _label;
      size_t                _flags;
      UsdAttribute          _attr;
  };

  // Graph connexion class
  //-------------------------------------------------------------------
  class Connexion {
    public:
      Connexion(Port* start, Port* end)
        : _start(start)
        , _end(end){};

      Port* GetStart() { return _start; };
      Port* GetEnd() { return _end; };

    protected:
      Port*               _start;
      Port*               _end;
  };


  // Graph node class
  //-------------------------------------------------------------------
  class Node {
    public: 
      Node(UsdPrim& prim);
      ~Node();

      void AddInput(UsdAttribute& attribute, const TfToken& name, 
        size_t flags=Port::INPUT|Port::HORIZONTAL);
      void AddOutput(UsdAttribute& attribute, const TfToken& name, 
        size_t flags=Port::OUTPUT|Port::HORIZONTAL);
      void AddPort(UsdAttribute& attribute, const TfToken& name, 
        size_t flags=Port::INPUT|Port::OUTPUT|Port::HORIZONTAL);

      size_t GetNumPorts() { return _ports.size(); };
      std::vector<Port>& GetPorts() { return _ports; };
      UsdPrim& GetPrim() { return _prim; };
      const UsdPrim& GetPrim() const { return _prim; };
      bool IsCompound();

      TfToken GetName() { return _name; };
      Port* GetPort(const TfToken& name);

      bool HasPort(const TfToken& name);

    protected:
      virtual void                _PopulatePorts() {};
      
      Node*                       _parent;
      TfToken                     _name;
      UsdPrim                     _prim;
      std::vector<Port>           _ports;
  };

public:
  Graph(const UsdPrim& prim);
  virtual ~Graph();

  virtual void Populate(const UsdPrim& prim);
  virtual void Clear();

  virtual void AddNode(Node* node);
  virtual void RemoveNode(Node* node);

  virtual void AddConnexion(Connexion* connexion);
  virtual void RemoveConnexion(Connexion* connexion);

  virtual short GetType() = 0;

  const std::vector<Node*>& GetNodes() const { return _nodes; };
  std::vector<Node*>& GetNodes() { return _nodes; };

  const Node* GetNode(const UsdPrim& prim) const;
  Node* GetNode(const UsdPrim& prim);
  
  const UsdPrim& GetPrim() const { return _prim; };
  UsdPrim& GetPrim() { return _prim; };

  const std::vector<Connexion*>& GetConnexions() const { return _connexions; };
  std::vector<Connexion*>& GetConnexions() { return _connexions; };

  bool ConnexionPossible(const Port* lhs, const Port* rhs);


protected:
  virtual void _DiscoverNodes() = 0;
  virtual void _DiscoverConnexions() = 0;
  
  std::vector<Node*>              _nodes;
  std::vector<Connexion*>         _connexions;
  UsdPrim                         _prim;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GRAPH_GRAPH_H