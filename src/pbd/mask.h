#ifndef JVR_PBD_MASK_H
#define JVR_PBD_MASK_H

#include "../common.h"
#include "../pbd/element.h"

JVR_NAMESPACE_OPEN_SCOPE

class Mask : public Element
{
public:
  static const size_t INVALID_INDEX = std::numeric_limits<int>::max();

  struct Iterator {
    Iterator(const Mask* mask, size_t begin, size_t end) 
      : _mask(mask)
      , _beg(begin)
      , _end(end)
      , _cur(0)
      , _values(nullptr)
      , _complete(true) {
      if (_mask->HasMask()) {
        _values = _mask->GetMaskCPtr();
        _complete = false;
      }
    };
    
    int Begin() {
      _cur = _beg - 1;
      return  Next();
    }

    int Next() {
      _cur++;
      if (_complete) return _cur < _end ? _cur : INVALID_INDEX;
      while(_cur < _end && BIT_CHECK(_values[_cur / sizeof(int)], _cur % sizeof(int)) != true) {
        _cur++;
      }
      return _cur < _end ? _cur : INVALID_INDEX;
    }

    const Mask*     _mask;
    const int*      _values;
    size_t          _beg;
    size_t          _end;
    size_t          _cur;
    bool            _complete;
    };

  enum Type {
    CONSTRAINT,
    COLLISION,
    FORCE
  };

  Mask(short type) : Element(type) {};
  Mask(short type, size_t n, const std::vector<int>& used) : Element(type) { SetMask(n, used); };

  bool HasMask() const { return _mask.size() > 0; };
  size_t MaskSize() const { return _mask.size(); };
  const int* GetMaskCPtr() const { return &_mask[0]; };
  void SetMask(size_t n, const std::vector<int>& used);
  void RemoveMask();

  bool HasWeights() const { return _weights.size() > 0; };
  void SetWeights(const std::vector<float>& weights) { _weights = weights; };
  void RemoveWeights() { _weights.clear(); };

protected:     
  size_t                      _size;          // unmasked particles size for this element
  std::vector<int>            _mask;          // used particles flag stored in bits
  std::vector<float>          _weights;       // if mask weights size must equals mask size 
                                              // else weights size must equals num particles
  friend class Particles;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_PBD_MASK_H
