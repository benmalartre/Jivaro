#ifndef JVR_APPLICATION_LAYOUT_H
#define JVR_APPLICATION_LAYOUT_H

#include <vector>
#include "../common.h"


JVR_NAMESPACE_OPEN_SCOPE
class View;
class Window;
class SplitterUI;
class Layout
{
public:
  enum Type
  {
    STANDARD,
    RAW
  };

  Layout();
  ~Layout();

  void Set(Window* window, short layout);
  void Resize(size_t width, size_t height);
  void Draw(bool forceRedraw = false);
  void SetDirty();

  SplitterUI* GetSplitter() { return _splitter; };
  const SplitterUI* GetSplitter() const { return _splitter; };

  View* GetView() { return _view; };
  const View* GetView() const { return _view; };

private:
  void _RawLayout(Window* window);
  void _StandardLayout(Window* window);
  void _Reset(Window* window);

  View*       _view;
  SplitterUI* _splitter;

  int         _width;
  int         _height;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_LAYOUT_VIEW_H