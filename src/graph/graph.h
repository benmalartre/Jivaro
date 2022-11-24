#ifndef JVR_GRAPH_GRAPH_H
#define JVR_GRAPH_GRAPH_H

#include <pxr/pxr.h>
#include "pxr/base/tf/type.h"
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include "../common.h"

JVR_NAMESPACE_OPEN_SCOPE


class Graph 
{
public:
  class Node;
  class Port;
  class Connexion;

  // Graph port class
  //-------------------------------------------------------------------
  class Port {
    public:
      enum Flag {
        INPUT = 1,
        OUTPUT = 2
      };

      Port() {};
      Port(Node* node, Flag flag, const pxr::TfToken& label, 
        pxr::UsdAttribute& attribute);

      bool IsInput() { return _flags & INPUT; };
      bool IsOutput() { return _flags & OUTPUT; };
      bool IsBothInputOutput() { return _flags & (INPUT | OUTPUT); };

      const pxr::TfToken& GetName()const {return _label;};
      pxr::SdfPath GetPath();
      const Node* GetNode() const { return _node; };
      Node* GetNode() { return _node; };
      void SetNode(Node* node) { _node = node; };
      const pxr::UsdAttribute& GetAttr() const { return _attr;};
      pxr::UsdAttribute& GetAttr() { return _attr;};
      Flag GetFlags() { return _flags; };
      pxr::TfToken GetLabel() { return _label; };
      bool IsConnected(Graph* graph, Connexion* foundConnexion);

    protected:
      Node*                 _node;
      pxr::TfToken          _label;
      Flag                  _flags;
      pxr::UsdAttribute     _attr;
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
      Node(pxr::UsdPrim& prim);
      ~Node();

      void AddInput(pxr::UsdAttribute& attribute, const pxr::TfToken& name);
      void AddOutput(pxr::UsdAttribute& attribute, const pxr::TfToken& name);
      void AddPort(pxr::UsdAttribute& attribute, const pxr::TfToken& name);

      size_t GetNumPorts() { return _ports.size(); };
      std::vector<Port>& GetPorts() { return _ports; };
      pxr::UsdPrim& GetPrim() { return _prim; };
      const pxr::UsdPrim& GetPrim() const { return _prim; };
      bool IsCompound();

      pxr::TfToken GetName() { return _name; };
      Port* GetPort(const pxr::TfToken& name);
      virtual pxr::GfVec2i GetPosition() { return pxr::GfVec2i(); };
      virtual pxr::GfVec2i GetSize() { return pxr::GfVec2i(); };
      virtual float GetWidth() { return 0.f; };
      virtual float GetHeight() { return 0.f; };

    protected:
      Node*                       _parent;
      pxr::TfToken                _name;
      pxr::UsdPrim                _prim;
      std::vector<Port>           _ports;
      short                       _dirty;
      pxr::GfVec2f                _pos;
      pxr::GfVec2f                _size;
      short                       _expended;
  };

public:
  Graph(pxr::UsdPrim& prim);
  ~Graph();

  virtual void Populate(pxr::UsdPrim& prim);
  virtual void Clear();

  virtual void AddNode(Node* node);
  virtual void RemoveNode(Node* node);

  virtual void AddConnexion(Connexion* connexion);
  virtual void RemoveConnexion(Connexion* connexion);

  const std::vector<Node*>& GetNodes() const { return _nodes; };
  std::vector<Node*>& GetNodes() { return _nodes; };

  const Node* GetNode(const pxr::UsdPrim& prim) const;
  Node* GetNode(const pxr::UsdPrim& prim);
  
  const pxr::UsdPrim& GetPrim() const { return _prim; };
  pxr::UsdPrim& GetPrim() { return _prim; };

  const std::vector<Connexion*>& GetConnexions() const { return _connexions; };
  std::vector<Connexion*>& GetConnexions() { return _connexions; };

  bool ConnexionPossible(const Port* lhs, const Port* rhs);

protected:
  void _DiscoverNodes(pxr::UsdPrim& prim);
  void _DiscoverConnexions(pxr::UsdPrim& prim);
  

  std::vector<Node*>              _nodes;
  std::vector<Connexion*>         _connexions;
  pxr::UsdPrim                    _prim;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_GRAPH_GRAPH_H