#include <QtGui>

class Chooser : public QWidget
{
public:
    Chooser(const QList<QByteArray> &choices, const QByteArray &delimiter)
        : QWidget(0), mCur(0)
    {
        QPalette p = palette();
        p.setBrush(backgroundRole(), Qt::black);
        setPalette(p);
        if (delimiter.isEmpty()) {
            mChoices = choices;
        } else {
            for (const QByteArray &choice : choices) {
                const int idx = choice.indexOf(delimiter);
                if (idx != -1) {
                    mChoices << choice.left(idx);
                    mData << choice.mid(idx + 1);
                } else {
                    mChoices << choice;
                }
            }
        }
        QFont f;
        f.setPixelSize(15);
        setFont(f);
        QFontMetrics fm(f);
        const int lineHeight = fm.height() + 4;
        const int h = mChoices.size() * lineHeight;
        int widest = 0;
        for (const QByteArray &c : mChoices) {
            widest = qMax(widest, fm.boundingRect(c).width());
        }
        setFixedSize(qMax(400, widest + 15), h + 15);
    }

    void paintEvent(QPaintEvent *e)
    {
        QPainter p(this);
        p.fillRect(rect(), Qt::black);
        p.setPen(mCur == -1 ? Qt::red : Qt::green);
        p.drawText(rect(), Qt::AlignLeft|Qt::AlignBottom, mInput);
        p.drawText(rect(), Qt::AlignRight|Qt::AlignBottom, mPattern);
        p.setPen(Qt::gray);
        const int lineHeight = QFontMetrics(p.font()).height() + 4;
        const int h = mChoices.size() * lineHeight;
        int y = (height() - h) / 2;
        for (int i=0; i<mChoices.size(); ++i) {
            const QRect r(0, y, width(), lineHeight);
            if (i == mCur) {
                p.save();
                p.setPen(Qt::black);
                QFont f = p.font();
                f.setBold(true);
                p.setFont(f);
                QFontMetrics fm(f);
                const QRect rr = fm.boundingRect(r, Qt::AlignCenter, mChoices.at(i));
                p.fillRect(rr, Qt::white);
            }

            p.drawText(0, y, width(), lineHeight, Qt::AlignCenter, mChoices.at(i));

            y += lineHeight;

            if (i == mCur)
                p.restore();
        }

        if (mCur != -1) {
            p.setPen(Qt::white);
        }
    }

    void keyPressEvent(QKeyEvent *e)
    {
        bool search = false;
        switch (e->key()) {
        case Qt::Key_Escape:
            if (mInput.isEmpty()) {
                close();
                return;
            }
            mInput.clear();
            update();
            break;
        case Qt::Key_Down:
            mInput.clear();
            if (mCur == -1) {
                mCur = 0;
            } else if (mCur + 1 < mChoices.size()) {
                ++mCur;
            } else {
                mCur = 0;
            }
            update();
            break;
        case Qt::Key_Up:
            mInput.clear();
            if (mCur == -1) {
                mCur = 0;
            } else if (mCur) {
                --mCur;
            } else {
                mCur = mChoices.size() - 1;
            }
            update();
            break;
        case Qt::Key_Backspace:
            mInput.chop(1);
            search = true;
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (mCur != -1) {
                QByteArray data = mData.value(mCur);
                if (data.isEmpty())
                    data = mChoices.at(mCur);
                printf("%s\n", data.constData());
                close();
            }
            return;
        case Qt::Key_W:
        case Qt::Key_C:
        case Qt::Key_Q:
            if (e->modifiers() & Qt::ControlModifier) {
                close();
                return;
            }
            break;
        }

        if (!e->text().isEmpty() && !e->modifiers() && isalnum(static_cast<unsigned char>(e->text().at(0).toAscii()))) {
            mInput.append(e->text());
            search = true;
        }
        if (search) {
            mPattern.clear();
            for (int i=0; i<mInput.size(); ++i) {
                if (i)
                    mPattern.append(".*");
                mPattern.append(mInput.at(i));
            }
            QRegExp rx(mPattern);
            rx.setCaseSensitivity(Qt::CaseInsensitive);
            mCur = -1;
            for (int i=0; i<mChoices.size(); ++i) {
                if (rx.indexIn(mChoices.at(i)) != -1) {
                    mCur = i;
                    break;
                }
            }

            update();
        }
    }
private:
    QByteArray mInput;
    QString mPattern;
    QList<QByteArray> mChoices;
    QList<QByteArray> mData;
    int mCur;
};

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    QList<QByteArray> choices;
    QByteArray delimiter;
    for (int i=1; i<argc; ++i) {
        if (!strcmp(argv[i], "--help")) {
            printf("qtchooser ([-d|--delimiter] delimiter) ...choices...\n");
            return 0;
        // } else if (!strcmp(argv[i], "--size") || !strcmp(argv[i], "-s")) {
        //     if (i + 1 == argc) {
        //         fprintf(stderr, "Tool!\n");
        //         return 1;
        //     }
        //     ++i;
        //     const char *x = strchr(argv[i], 'x');
        //     if (!x) {
        //         fprintf(stderr, "Tool!\n");
        //         return 1;
        //     }
        //     bool ok;
        //     const int w = QByteArray(argv[i], x - argv[i]).toUInt(&ok);
        //     if (!ok) {
        //         fprintf(stderr, "Tool!\n");
        //         return 1;
        //     }
        //     const int h = QByteArray(argv[i], x - argv[i]).toUInt(&ok);
        //     if (!ok) {
        //         fprintf(stderr, "Tool!\n");
        //         return 1;
        //     }
        //     size = QSize(w, h);
        } else if (!strcmp(argv[i], "--delimiter") || !strcmp(argv[i], "-d")) {
            if (i + 1 == argc) {
                fprintf(stderr, "Tool!\n");
                return 1;
            }
            delimiter = argv[++i];
        } else if (!strcmp(argv[i], "--")) {
            char buf[16384];
            while ((fgets(buf, sizeof(buf), stdin))) {
                QByteArray str = buf;
                choices << str.trimmed();
            }
        } else {
            choices << argv[i];
        }
    }

    Chooser c(choices, delimiter);
    // c.setFixedSize(size);
    c.show();
    return a.exec();
}
