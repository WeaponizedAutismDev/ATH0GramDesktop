void LinksWidget::setupControls() {
    _uniqueToggle = std::make_unique<Ui::ToggleView>(
        st::infoMediaLinksToggle,
        false,
        [=] { updateUniqueState(); });
    _uniqueToggle->setGeometry(0, 0, st::infoMediaLinksToggle.width, st::infoMediaLinksToggle.height);
    _uniqueToggle->show();

    _exportButton = std::make_unique<Ui::IconButton>(
        this,
        st::infoMediaLinksExport);
    _exportButton->setClickedCallback([=] { exportLinks(); });
    _exportButton->show();

    // Add scam detection
    _scamDetection = std::make_unique<ScamDetection>(this);
}

void LinksWidget::updateUniqueState() {
    const auto unique = _uniqueToggle->checked();
    if (_showUnique == unique) {
        return;
    }
    _showUnique = unique;
    refreshLinks();
}

void LinksWidget::exportLinks() {
    const auto links = collectLinks();
    if (links.empty()) {
        return;
    }

    const auto filters = QString("JSON Files (*.json);;CSV Files (*.csv)");
    const auto path = QFileDialog::getSaveFileName(
        this,
        tr::lng_media_links_export_title(tr::now),
        QString(),
        filters);

    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    if (path.endsWith(".json", Qt::CaseInsensitive)) {
        exportToJson(file, links);
    } else {
        exportToCsv(file, links);
    }
}

void LinksWidget::exportToJson(QFile &file, const QVector<LinkData> &links) {
    QJsonArray array;
    for (const auto &link : links) {
        QJsonObject obj;
        obj["url"] = link.url;
        obj["title"] = link.title;
        obj["description"] = link.description;
        obj["date"] = link.date.toString(Qt::ISODate);
        obj["isScam"] = link.isScam;
        array.append(obj);
    }

    QJsonDocument doc(array);
    file.write(doc.toJson());
}

void LinksWidget::exportToCsv(QFile &file, const QVector<LinkData> &links) {
    QTextStream stream(&file);
    stream << "URL,Title,Description,Date,IsScam\n";
    
    for (const auto &link : links) {
        stream << "\"" << link.url << "\","
               << "\"" << link.title << "\","
               << "\"" << link.description << "\","
               << "\"" << link.date.toString(Qt::ISODate) << "\","
               << (link.isScam ? "true" : "false") << "\n";
    }
}

void LinksWidget::refreshLinks() {
    const auto links = collectLinks();
    _list->clear();

    QSet<QString> uniqueUrls;
    for (const auto &link : links) {
        if (_showUnique && uniqueUrls.contains(link.url)) {
            continue;
        }
        uniqueUrls.insert(link.url);

        auto item = std::make_unique<LinkItem>(link);
        if (link.isScam) {
            item->setScamBadge();
        }
        _list->addItem(std::move(item));
    }
}

bool ScamDetection::isScamLink(const QString &url) const {
    return isDisguisedLink(url) || isCryptoScam(url) || isExternalSpam(url);
}

QString ScamDetection::getRealUrl(const QString &url) const {
    return resolveRedirect(url);
}

bool ScamDetection::isDisguisedLink(const QString &url) const {
    // Check for t.me links that redirect to external sites
    if (url.startsWith("https://t.me/")) {
        const auto realUrl = resolveRedirect(url);
        return !realUrl.startsWith("https://t.me/") && !realUrl.startsWith("@");
    }
    return false;
}

bool ScamDetection::isCryptoScam(const QString &url) const {
    // Common patterns in crypto scam URLs
    const QStringList cryptoPatterns = {
        "crypto", "token", "airdrop", "presale", "ico", "defi",
        "staking", "mining", "wallet", "exchange", "trading"
    };

    const auto realUrl = resolveRedirect(url).toLower();
    for (const auto &pattern : cryptoPatterns) {
        if (realUrl.contains(pattern)) {
            // Additional checks for common scam indicators
            if (realUrl.contains("?start=") || realUrl.contains("?ref=")) {
                return true;
            }
        }
    }
    return false;
}

bool ScamDetection::isExternalSpam(const QString &url) const {
    // Check for common spam domains and patterns
    const QStringList spamPatterns = {
        "bit.ly", "goo.gl", "tinyurl", "t.co", "ow.ly",
        "adf.ly", "shorte.st", "bc.vc", "adfly"
    };

    const auto realUrl = resolveRedirect(url).toLower();
    for (const auto &pattern : spamPatterns) {
        if (realUrl.contains(pattern)) {
            return true;
        }
    }
    return false;
}

QString ScamDetection::resolveRedirect(const QString &url) const {
    // In a real implementation, this would follow redirects
    // For now, we'll just check for common patterns
    if (url.contains("?start=") || url.contains("?ref=")) {
        const auto parts = url.split('?');
        if (parts.size() > 1) {
            return parts[0];
        }
    }
    return url;
}

LinkTracker::LinkTracker() {
    _regexCache.setMaxCost(kRegexCacheSize);
    _cleanupTimer.setInterval(24 * 60 * 60 * 1000); // 24 hours
    connect(&_cleanupTimer, &QTimer::timeout, this, &LinkTracker::cleanup);
    _cleanupTimer.start();
    loadState();
}

LinkTracker::~LinkTracker() {
    saveState();
}

void LinkTracker::trackVisit(const QString &url) {
    if (_isProcessingBatch) {
        _batchQueue.enqueue(url);
        if (_batchQueue.size() >= kBatchSize) {
            processBatch();
        }
        return;
    }

    const auto now = QDateTime::currentDateTime();
    checkMemoryUsage();
    
    if (!_visitedLinks.value(url, false)) {
        _visitedLinks[url] = true;
        saveState();
    }
    _lastSeen[url] = now;
}

void LinkTracker::processBatch() {
    if (_batchQueue.isEmpty()) {
        _isProcessingBatch = false;
        return;
    }

    _isProcessingBatch = true;
    const auto now = QDateTime::currentDateTime();
    checkMemoryUsage();

    for (int i = 0; i < kBatchSize && !_batchQueue.isEmpty(); ++i) {
        const auto url = _batchQueue.dequeue();
        if (!_visitedLinks.value(url, false)) {
            _visitedLinks[url] = true;
        }
        _lastSeen[url] = now;
    }

    saveState();
    _isProcessingBatch = false;

    if (!_batchQueue.isEmpty()) {
        QTimer::singleShot(0, this, &LinkTracker::processBatch);
    }
}

QRegularExpression* LinkTracker::getCachedRegex(const QString &pattern) {
    if (auto cached = _regexCache.object(pattern)) {
        return cached;
    }

    auto regex = new QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
    _regexCache.insert(pattern, regex);
    return regex;
}

void LinkTracker::checkMemoryUsage() {
    const auto totalLinks = _visitedLinks.size() + _memberLinks.size() + _channelNames.size();
    if (totalLinks > kMaxTrackedLinks) {
        // Remove oldest entries
        auto oldest = _lastSeen.begin();
        for (auto it = _lastSeen.begin(); it != _lastSeen.end(); ++it) {
            if (it.value() < oldest.value()) {
                oldest = it;
            }
        }
        if (oldest != _lastSeen.end()) {
            const auto url = oldest.key();
            _visitedLinks.remove(url);
            _memberLinks.remove(url);
            _channelNames.remove(url);
            _lastSeen.erase(oldest);
        }
    }
}

void LinkTracker::saveState() {
    Core::App().settings().setVisitedLinks(_visitedLinks);
    Core::App().settings().setLinkLastSeen(_lastSeen);
}

void LinkTracker::loadState() {
    _visitedLinks = Core::App().settings().getVisitedLinks();
    _lastSeen = Core::App().settings().getLinkLastSeen();
}

void LinkTracker::backupData() {
    const auto path = QFileDialog::getSaveFileName(
        nullptr,
        tr::lng_media_links_backup_title(tr::now),
        Core::App().settings().getDefaultBackupPath(),
        "JSON Files (*.json)");
    
    if (!path.isEmpty()) {
        // Check disk space
        QStorageInfo storage = QStorageInfo::root();
        if (storage.bytesAvailable() < kMaxBackupSize) {
            Ui::show(Box<InformBox>(
                tr::lng_media_links_backup_no_space(tr::now),
                tr::lng_media_links_backup_no_space_details(tr::now)));
            return;
        }

        // Check file permissions
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            Ui::show(Box<InformBox>(
                tr::lng_media_links_backup_error(tr::now),
                tr::lng_media_links_backup_error_details(tr::now)));
            return;
        }

        // Show progress
        auto progress = Ui::show(Box<ProgressBox>(
            tr::lng_media_links_backup_progress(tr::now)));
        
        // Perform backup in background
        QTimer::singleShot(0, [=] {
            try {
                Core::App().settings().backupLinkData(path);
                progress->closeBox();
                Ui::show(Box<InformBox>(tr::lng_media_links_backup_success(tr::now)));
            } catch (const std::exception &e) {
                progress->closeBox();
                Ui::show(Box<InformBox>(
                    tr::lng_media_links_backup_error(tr::now),
                    QString("Error: %1").arg(e.what())));
            }
        });
    }
}

void LinkTracker::restoreData() {
    const auto path = QFileDialog::getOpenFileName(
        nullptr,
        tr::lng_media_links_restore_title(tr::now),
        Core::App().settings().getDefaultBackupPath(),
        "JSON Files (*.json)");
    
    if (!path.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            Ui::show(Box<InformBox>(
                tr::lng_media_links_restore_error(tr::now),
                tr::lng_media_links_restore_error_details(tr::now)));
            return;
        }

        // Check file size
        if (file.size() > kMaxBackupSize) {
            Ui::show(Box<InformBox>(
                tr::lng_media_links_restore_too_large(tr::now),
                tr::lng_media_links_restore_too_large_details(tr::now)));
            return;
        }

        // Show progress
        auto progress = Ui::show(Box<ProgressBox>(
            tr::lng_media_links_restore_progress(tr::now)));
        
        // Perform restore in background
        QTimer::singleShot(0, [=] {
            try {
                // Validate JSON
                const auto doc = QJsonDocument::fromJson(file.readAll());
                if (!doc.isObject()) {
                    throw std::runtime_error("Invalid backup file format");
                }

                Core::App().settings().restoreLinkData(path);
                progress->closeBox();
                Ui::show(Box<InformBox>(tr::lng_media_links_restore_success(tr::now)));
            } catch (const std::exception &e) {
                progress->closeBox();
                Ui::show(Box<InformBox>(
                    tr::lng_media_links_restore_error(tr::now),
                    QString("Error: %1").arg(e.what())));
            }
        });
    }
}

bool LinkTracker::isVisited(const QString &url) const {
    return _visitedLinks.value(url, false);
}

bool LinkTracker::isMember(const QString &url) const {
    return _memberLinks.value(url, false);
}

void LinkTracker::setMember(const QString &url, bool isMember) {
    if (_memberLinks.value(url, false) != isMember) {
        _memberLinks[url] = isMember;
        Core::App().settings().setMemberLinks(_memberLinks);
    }
}

QString LinkTracker::getChannelName(const QString &url) const {
    return _channelNames.value(url);
}

void LinkTracker::setChannelName(const QString &url, const QString &name) {
    if (_channelNames.value(url) != name) {
        _channelNames[url] = name;
        Core::App().settings().setChannelNames(_channelNames);
    }
}

LinkItem::LinkItem(const LinkData &data, QWidget *parent)
    : RippleButton(parent)
    , _data(data) {
    setCursor(style::cur_pointer);
}

void LinkItem::setVisited(bool visited) {
    if (_visited != visited) {
        _visited = visited;
        update();
    }
}

void LinkItem::setMember(bool member) {
    if (_member != member) {
        _member = member;
        update();
    }
}

void LinkItem::setChannelName(const QString &name) {
    if (_channelName != name) {
        _channelName = name.left(kMaxChannelNameLength);
        update();
    }
}

void LinkItem::paintEvent(QPaintEvent *e) {
    Painter p(this);
    const auto &st = st::infoMediaLinksItem;

    // Draw background
    p.fillRect(rect(), st::windowBg);
    if (_ripple) {
        _ripple->paint(p, 0, 0, width());
    }

    // Draw link info
    const auto left = st.padding.left();
    const auto top = st.padding.top();
    const auto availableWidth = width() - left - st.padding.right();

    // Draw URL with visited/member status
    auto urlText = _data.url;
    if (!_channelName.isEmpty()) {
        urlText = _channelName;
    }
    
    p.setPen(_visited ? st::windowSubTextFg : st::windowTextFg);
    if (_member) {
        p.setFont(st::semiboldFont);
    }
    p.drawTextLeft(left, top, width(), urlText);

    // Draw status indicators
    if (_visited) {
        const auto check = st::infoMediaLinksCheck;
        p.drawPixmap(
            width() - check.width() - st.padding.right(),
            (height() - check.height()) / 2,
            check);
    }
    if (_member) {
        const auto member = st::infoMediaLinksMember;
        p.drawPixmap(
            width() - member.width() - st.padding.right() - (_visited ? check.width() + st.padding.right() : 0),
            (height() - member.height()) / 2,
            member);
    }
} 