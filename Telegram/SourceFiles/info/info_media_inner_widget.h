#pragma once

#include "info/info_content_widget.h"
#include "ui/widgets/scroll_area.h"
#include "ui/widgets/buttons.h"
#include "data/data_shared_media.h"

namespace Ui {
class IconButton;
} // namespace Ui

namespace Info {
namespace Media {

class InnerWidget : public Ui::RpWidget {
public:
    // ... existing code ...
    
    void rebuildMediaIndex();
    void showRefreshButton();
    void hideRefreshButton();
    
private:
    // ... existing code ...
    object_ptr<Ui::IconButton> _refreshButton;
    bool _isRebuilding = false;
    
    void setupRefreshButton();
    void onRefreshClicked();
    
    // ... existing code ...
};

} // namespace Media
} // namespace Info 