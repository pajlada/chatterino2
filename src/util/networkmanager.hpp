#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>
#include <QUrl>

namespace chatterino {

namespace util {

class NetworkWorker : public QObject
{
    Q_OBJECT

signals:
    void doneUrl(QNetworkReply *);
};

class NetworkRequester : public QObject
{
    Q_OBJECT

signals:
    void requestUrl();
};

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    static QThread workerThread;
    static QNetworkAccessManager NaM;

    static void init();
    static void deinit();

    template <typename FinishedCallback>
    static void urlFetch(QNetworkRequest request, FinishedCallback onFinished)
    {
        NetworkRequester requester;
        NetworkWorker *worker = new NetworkWorker;

        worker->moveToThread(&NetworkManager::workerThread);
        QObject::connect(
            &requester, &NetworkRequester::requestUrl, worker,
            [ worker, onFinished{std::move(onFinished)}, request{std::move(request)} ]() {
                QNetworkReply *reply = NetworkManager::NaM.get(request);

                QObject::connect(reply, &QNetworkReply::finished, [onFinished, reply, worker]() {
                    onFinished(reply);
                    delete worker;
                });
            });

        emit requester.requestUrl();
    }

    template <typename FinishedCallback>
    static void urlFetch(const QUrl &url, FinishedCallback onFinished)
    {
        urlFetch(QNetworkRequest(url), std::move(onFinished));
    }

    template <typename FinishedCallback, typename ConnectedCallback = void (*)(QNetworkReply *)>
    static void urlFetch(QNetworkRequest request, const QObject *caller,
                         FinishedCallback onFinished,
                         ConnectedCallback onConnected = [](QNetworkReply *) { return; })
    {
        NetworkRequester requester;
        NetworkWorker *worker = new NetworkWorker;

        worker->moveToThread(&NetworkManager::workerThread);

        QObject::connect(&requester, &NetworkRequester::requestUrl, worker, [=]() {
            QNetworkReply *reply = NetworkManager::NaM.get(request);

            onConnected(reply);

            QObject::connect(reply, &QNetworkReply::finished, worker, [worker, reply]() {
                emit worker->doneUrl(reply);  //
                // TODO: Do we need to mark the worker as "to be deleted" here?
            });
        });

        QObject::connect(worker, &NetworkWorker::doneUrl, caller, [=](QNetworkReply *reply) {
            onFinished(reply);
            delete worker;
        });
        emit requester.requestUrl();
    }

    template <typename FinishedCallback, typename ConnectedCallback = void (*)(QNetworkReply *)>
    static void urlFetch(const QUrl &url, const QObject *caller, FinishedCallback onFinished,
                         ConnectedCallback onConnected = [](QNetworkReply *) { return; })
    {
        urlFetch(QNetworkRequest(url), caller, onFinished, onConnected);
    }

    template <typename FinishedCallback>
    static void urlPut(QNetworkRequest request, FinishedCallback onFinished, QByteArray *data)
    {
        NetworkRequester requester;
        NetworkWorker *worker = new NetworkWorker;

        worker->moveToThread(&NetworkManager::workerThread);
        QObject::connect(
            &requester, &NetworkRequester::requestUrl, worker,
            [ worker, onFinished{std::move(onFinished)}, request{std::move(request)}, data ]() {
                QNetworkReply *reply = NetworkManager::NaM.put(request, *data);

                QObject::connect(reply, &QNetworkReply::finished,
                                 [ worker, onFinished = std::move(onFinished), reply ]() {
                                     onFinished(reply);
                                     delete worker;
                                 });
            });

        emit requester.requestUrl();
    }

    template <typename FinishedCallback>
    static void urlPut(QNetworkRequest request, FinishedCallback onFinished)
    {
        NetworkRequester requester;
        NetworkWorker *worker = new NetworkWorker;

        worker->moveToThread(&NetworkManager::workerThread);
        QObject::connect(
            &requester, &NetworkRequester::requestUrl, worker,
            [ onFinished{std::move(onFinished)}, request{std::move(request)}, worker ]() {
                QNetworkReply *reply = NetworkManager::NaM.put(request, "");

                QObject::connect(reply, &QNetworkReply::finished,
                                 [ onFinished{std::move(onFinished)}, reply, worker ]() {
                                     onFinished(reply);
                                     delete worker;
                                 });
            });

        emit requester.requestUrl();
    }
};

}  // namespace util
}  // namespace chatterino
