#ifndef JVR_EXEC_NODE_H
#define JVR_EXEC_NODE_H

class ExecNode : Graph::Node {
public:
    virtual void Compute(ExecContext& context) = 0;
    virtual void Save();
    virtual void Read();

    static bool IsExecNode(const UsdPrim &prim);

private:
    UsdPrim                     _prim;  
    std::vector<UsdAttribute>   _attributes;
    std::vector<Graph::Port*>   _ports;
};

#endif