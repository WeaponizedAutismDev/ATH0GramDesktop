#include "info/info_media_inner_widget.h"
#include "info/info_media_list_widget.h"
#include "ui/widgets/buttons.h"
#include "styles/style_info.h"

namespace Info {
namespace Media {

void InnerWidget::setupList() {
    // ... existing code ...
    
    // Add refresh button
    _refreshButton = object_ptr<Ui::IconButton>(
        this,
        st::infoMediaRefreshButton);
    _refreshButton->setClickedCallback([this] {
        rebuildMediaIndex();
    });
    _refreshButton->hide();
    
    // ... existing code ...
}

void InnerWidget::rebuildMediaIndex() {
    if (_isRebuilding) {
        return;
    }
    
    _isRebuilding = true;
    _refreshButton->hide();
    
    // Show loading state
    _list->showLoading();
    
    // Rebuild the media index
    if (const auto viewer = _list->sharedMediaViewer()) {
        viewer->rebuildIndex();
    }
    
    // Reload the media list
    _list->loadMore();
    
    _isRebuilding = false;
    _refreshButton->show();
}

void InnerWidget::showRefreshButton() {
    if (!_isRebuilding) {
        _refreshButton->show();
    }
}

void InnerWidget::hideRefreshButton() {
    _refreshButton->hide();
}

} // namespace Media
} // namespace Info 