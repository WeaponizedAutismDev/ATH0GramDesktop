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

class LinkTracker : public QObject {
    Q_OBJECT

public:
    explicit LinkTracker(QObject *parent = nullptr);
    
    void trackVisit(const QString &url);
    bool isVisited(const QString &url) const;
    bool isMember(const QString &url) const;
    void setMember(const QString &url, bool isMember);
    QString getChannelName(const QString &url) const;
    void setChannelName(const QString &url, const QString &name);

private:
    QMap<QString, bool> _visitedLinks;
    QMap<QString, bool> _memberLinks;
    QMap<QString, QString> _channelNames;
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