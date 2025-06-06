#pragma once

#include "base/object_ptr.h"
#include "ui/effects/ripple_animation.h"
#include "ui/widgets/scroll_area.h"
#include "ui/widgets/buttons.h"
#include "core/settings.h"
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileDialog>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QCache>

namespace Ui {
class RippleButton;
class ToggleView;
class IconButton;
} // namespace Ui

namespace Info {
namespace Media {

constexpr int kMaxTrackedLinks = 10000;
constexpr int kMaxBackupSize = 10 * 1024 * 1024; // 10MB
constexpr int kMaxChannelNameLength = 100;
constexpr int kBatchSize = 100; // Process links in batches
constexpr int kRegexCacheSize = 100; // Cache size for regex patterns

class ScamDetection : public QObject {
    Q_OBJECT

public:
    explicit ScamDetection(QObject *parent = nullptr);

    bool isScamLink(const QString &url) const;
    QString getRealUrl(const QString &url) const;

private:
    bool isDisguisedLink(const QString &url) const;
    bool isCryptoScam(const QString &url) const;
    bool isExternalSpam(const QString &url) const;
    QString resolveRedirect(const QString &url) const;
};

struct LinkData {
    QString url;
    QString title;
    QString description;
    QDateTime date;
    bool isScam = false;
};

class LinksWidget : public RpWidget {
    Q_OBJECT

public:
    explicit LinksWidget(QWidget *parent);

private:
    void setupControls();
    void updateUniqueState();
    void exportLinks();
    void exportToJson(QFile &file, const QVector<LinkData> &links);
    void exportToCsv(QFile &file, const QVector<LinkData> &links);
    void refreshLinks();
    QVector<LinkData> collectLinks() const;

    std::unique_ptr<Ui::ToggleView> _uniqueToggle;
    std::unique_ptr<Ui::IconButton> _exportButton;
    std::unique_ptr<ScamDetection> _scamDetection;
    std::unique_ptr<Ui::ScrollArea> _list;
    bool _showUnique = false;
};

class LinkTracker {
public:
    LinkTracker();
    ~LinkTracker();

    void trackVisit(const QString &url);
    void trackMembership(const QString &url, bool isMember);
    void setChannelName(const QString &url, const QString &name);
    bool isVisited(const QString &url) const;
    bool isMember(const QString &url) const;
    QString getChannelName(const QString &url) const;
    void cleanup();
    void backupData();
    void restoreData();
    void exportData(const QString &format);

private:
    QHash<QString, bool> _visitedLinks;
    QHash<QString, bool> _memberLinks;
    QHash<QString, QString> _channelNames;
    QHash<QString, QDateTime> _lastSeen;
    QCache<QString, QRegularExpression> _regexCache;
    QTimer _cleanupTimer;
    bool _isProcessingBatch = false;
    QQueue<QString> _batchQueue;

    void processBatch();
    QRegularExpression* getCachedRegex(const QString &pattern);
    void saveState();
    void loadState();
    void checkMemoryUsage();
};

class LinkItem : public Ui::RippleButton {
public:
    LinkItem(const LinkData &data, QWidget *parent);

    void setVisited(bool visited);
    void setMember(bool member);
    void setChannelName(const QString &name);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    LinkData _data;
    bool _visited = false;
    bool _member = false;
    QString _channelName;
    std::unique_ptr<Ui::RippleAnimation> _ripple;
};

} // namespace Media
} // namespace Info 