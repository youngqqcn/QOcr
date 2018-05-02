#include "qocr.h"
#include "ui_qocr.h"

#include <openssl/aes.h>
#include <curl/curl.h>
#include <json/json.h>
#include <fstream>

#include <map>
#include <string>
#include <list>
#include <iterator>

#include "capturescreen.h"  //截图
#include "ocr.h" //文字识别

QOcr::QOcr(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QOcr)
{
    ui->setupUi(this);

    connect(this, SIGNAL(signalSaveImageComplete()), this, SLOT(TestAIP())); // 保存图片
}

QOcr::~QOcr()
{
    delete ui;
}

void QOcr::RecvSignalArgs(QString strAppId, QString strApiKey, QString strSecretKey)
{
    this->m_strAppId = strAppId;
    this->m_strApiKey= strApiKey;
    this->m_strSecretKey = strSecretKey;

    //TestJsonCpp();
    //TestAIP();
    //TestCurl();


    /*
    qDebug() << this->m_strAppId << endl;
    qDebug() << this->m_strApiKey << endl;
    qDebug() << this->m_strSecretKey << endl;
    */
}

void QOcr::RecvSingalExit()
{
    this->close(); //退出程序
}

void QOcr::on_pushButton_clicked()
{
    // 点击截图按钮开始截图;
    CaptureScreen* captureHelper = new CaptureScreen();
    connect(captureHelper, SIGNAL(signalCompleteCature(QPixmap)), this, SLOT(onCompleteCature(QPixmap)));
    connect(captureHelper, SIGNAL(signalCompleteCature(QPixmap)), this, SLOT(SaveImage(QPixmap))); // 保存图片
    captureHelper->show();
}

void QOcr::onCompleteCature(QPixmap captureImage)
{
    ui->label->setPixmap(captureImage);
}

void QOcr::SaveImage(QPixmap captureImage)
{
    QString strPath =  QCoreApplication::applicationDirPath() + "/tmp1.jpg";
    m_strImgPath = strPath.toStdString();
    //qDebug() << strPath << endl;
    bool bRet = captureImage.save(strPath);
    if(bRet)
    {
        signalSaveImageComplete();
    }
    else
    {
        //是否需要提示重新截图??
    }
}

static size_t downloadCallback(void *buffer, size_t sz, size_t nmemb, void *writer)
{
    std::string* psResponse = (std::string*)writer;
    size_t size = sz * nmemb;
    psResponse->append((char*)buffer, size);

    return sz * nmemb;
}

void QOcr::TestCurl()
{
#if 0
    CURL *curl = curl_easy_init();
    if (curl)
        qDebug() << "curl_easy_init() succeeded!\n" << endl;
    else
        qDebug() << "Error calling curl_easy_init().\n" << endl;
#endif

    //std::string strUrl = "http://www.baidu.com";
    std::string strUrl = "https://www.cnblogs.com/yangxunpeng/articles/7041088.html";
    std::string strTmpStr;
    CURL *curl = curl_easy_init();
    if(!curl) return;
    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloadCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strTmpStr);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);//设定为不验证证书和HOST
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    std::string strRsp;
    if (CURLE_UNSUPPORTED_PROTOCOL == res)
    {
        std::cout << "not support https" << std::endl;
    }
    else if (res != CURLE_OK)
    {
        strRsp = "error";
    }
    else
    {
        strRsp = strTmpStr;
        qDebug() << strRsp.c_str() << endl;
    }
}

void QOcr::TestJsonCpp()
{
    //字符串
    const char* str =
            "{\"praenomen\":\"Gaius\",\"nomen\":\"Julius\",\"cognomen\":\"Caezar\",\"born\":-100,\"died\":-44}";

    /*
      // json内容如下：
      {
            "praenomen":"Gaius",
            "nomen":"Julius",
            "cognomen":"Caezar",
            "born":-100,
            "died":-44
        }
      */

    Json::Reader reader;
    Json::Value root;

    //从字符串中读取数据
    if(reader.parse(str,root))
    {
        std::string praenomen = root["praenomen"].asString();
        std::string nomen = root["nomen"].asString();
        std::string cognomen = root["cognomen"].asString();
        int born = root["born"].asInt();
        int died = root["died"].asInt();

        qDebug() <<QString(praenomen.c_str()) + " " + nomen.c_str() + " " + cognomen.c_str()
                << " was born in year " << born
                << ", died in year " << died << endl;
    }

}

void QOcr::TestAIP()
{

    // 设置APPID/AK/SK
    /*
            * APP_ID = '9851066'
            * API_KEY = 'LUGBatgyRGoerR9FZbV4SQYk'
            * SECRET_KEY = 'fB2MNz1c2UHLTximFlC4laXPg7CVfyjV'
            */
    std::string app_id = "11176125";
    std::string api_key = "nUKlV0kDnZTBzNDBsONDhCXu";
    std::string secret_key = "5riESvRvtMLHhe9SM3sSMCt87E4bCapM";

    aip::Ocr client(app_id, api_key, secret_key);

    Json::Value result;

    std::string image;
    //aip::get_file_content("C:\\Users\\yqq\\Documents\\QtProjects\\QOcr\\img2.jpg", &image);
    aip::get_file_content(m_strImgPath.c_str(), &image);

    // 如果有可选参数
    std::map<std::string, std::string> options;
    options["language_type"] = "CHN_ENG";
    options["detect_direction"] = "true";
    options["detect_language"] = "true";
    options["probability"] = "true";

    result = client.accurate_basic(image, options);

    //std::cout << result["words_result_num"].asInt();
    //std::cout << result["words_result"].size() << std::endl;
    qDebug() << result["words_result"].size() << endl;

    ui->OcrOutput->clear();
    for (int i = 0; i < result["words_result"].size(); i++)
    {
        //std::cout << result["words_result"][i]["words"].asCString() << std::endl;
        std::string tmpStr = result["words_result"][i]["words"].asCString();
        qDebug() << QString(tmpStr.c_str()) << endl;
        ui->OcrOutput->append(QString(tmpStr.c_str()));
    }

}




void QOcr::TestAES()
{
    //char* text = "1234567890ABCDEF[]";
    //char *text = QString("yqq").toStdString().c_str();
    char *text = new char[strlen(QString("yqq").toStdString().c_str()) + 1];
    strcpy(text, QString("yqq").toStdString().c_str());
    size_t len = strlen(text) + 1;
    if (len % AES_BLOCK_SIZE != 0)
    {
        len = (len / AES_BLOCK_SIZE + 1)*AES_BLOCK_SIZE;  //len取整
    }

    //存放明文的字节数组
    unsigned char* plainText = new unsigned char[len];
    strcpy((char*)plainText, text);

    //128-bit的秘钥字节数组，占16个字节。可以存放任意值。
    unsigned char my_16bytes_key[16] = { 0 };
    for (int i = 0; i<16; ++i)
    {
        my_16bytes_key[i] = 0x30;
    }

    //生成openssl库使用的秘钥结构体
    AES_KEY aes_key;
    if (AES_set_encrypt_key(my_16bytes_key, 128, &aes_key) < 0)
    {
        //fprintf(stderr, "Unable to set encryption key in AES\n");
        qDebug() << "Unable to set encryption key in AES\n" << endl;
        exit(-1);
    }

    //初始向量，实际上就是一个16字节的字节序列，可以是任意值。
    unsigned char iv[AES_BLOCK_SIZE];        // init vector
    for (int i = 0; i<AES_BLOCK_SIZE; ++i)
    {
        iv[i] = 0;
    }

    //加密。加密的结果被存放到cyperText。cyperText的长度应该等于plainText的长度
    //秘钥和初始向量也是必要的参数。
    unsigned char* cyperText = new unsigned char[len];
    AES_cbc_encrypt(plainText, cyperText, len, &aes_key, iv, AES_ENCRYPT);

    //存放解密后的明文数据。因为明文和密文长度一致，当然知道解密后也要用len字节的内存。
    unsigned char* decryptText = new unsigned char[len];

    //初始向量。应与加密时的初始向量一致。
    //这里再次给iv数组赋值的原因：前一次调用AES_cbc_encrypt后，iv的内容已被改变了。这里是恢复为期望的值。
    for (int i = 0; i<AES_BLOCK_SIZE; ++i) {
        iv[i] = 0;
    }

    //初始化秘钥结构体。应该与加密时的秘钥相同，这才叫“对称加密”
    if (AES_set_decrypt_key(my_16bytes_key, 128, &aes_key) < 0) {
        fprintf(stderr, "Unable to set decryption key in AES\n");
        exit(-1);
    }

    //解密。
    AES_cbc_encrypt(cyperText, decryptText, len, &aes_key, iv, AES_DECRYPT);

    // print
    qDebug() << "plainText =" << QString((char *)plainText) << endl;
    QString strCyperText =  "cyperText = ";
    for (int i = 0; i<len; ++i) {
        char buf[10] = {0};
        sprintf(buf, "%x%x", (cyperText[i] >> 4) & 0xf, cyperText[i] & 0xf);
        strCyperText += buf;
    }
    qDebug() << strCyperText << endl;

    qDebug() << "decrypertext = " << QString((char *) decryptText) << endl;

}

void QOcr::Encode(char *plainStr, unsigned char **cryper)
{
    //char* text = "1234567890ABCDEF[]";
    char* text = plainStr;
    size_t len = strlen(text) + 1;
    if (len % AES_BLOCK_SIZE != 0) {
        len = (len / AES_BLOCK_SIZE + 1)*AES_BLOCK_SIZE;  //len取整
    }

    //存放明文的字节数组
    unsigned char* plainText = new unsigned char[len];
    strcpy((char*)plainText, text);

    //128-bit的秘钥字节数组，占16个字节。可以存放任意值。
    unsigned char my_16bytes_key[16] = { 0 };
    for (int i = 0; i < 16; ++i) {
        my_16bytes_key[i] = 0x30;
    }

    //生成openssl库使用的秘钥结构体
    AES_KEY aes_key;
    if (AES_set_encrypt_key(my_16bytes_key, 128, &aes_key) < 0) {
        fprintf(stderr, "Unable to set encryption key in AES\n");
        exit(-1);
    }

    //初始向量，实际上就是一个16字节的字节序列，可以是任意值。
    unsigned char iv[AES_BLOCK_SIZE];        // init vector
    for (int i = 0; i < AES_BLOCK_SIZE; ++i) {
        iv[i] = 0;
    }

    //加密。加密的结果被存放到cyperText。cyperText的长度应该等于plainText的长度
    //秘钥和初始向量也是必要的参数。
    //unsigned char* cyperText = new unsigned char[len];
    *cryper = new unsigned char[len];
    AES_cbc_encrypt(plainText, *cryper, len, &aes_key, iv, AES_ENCRYPT);

    for (int i = 0; i < len; ++i)
    {
        printf("%x%x", ((*cryper)[i] >> 4) & 0xf,
               (*cryper)[i] & 0xf);
    }
    printf("\n");

    std::ofstream ofs;
    ofs.open("password.dat", std::ios_base::out | std::ios_base::binary);

    for (int i = 0; i < len; ++i)
    {
        ofs << (*cryper)[i] ;
    }
    ofs.close();

    printf("\n");

}

void QOcr::Decode(unsigned char **cryper, char *plainStr)
{
    //128-bit的秘钥字节数组，占16个字节。可以存放任意值。
    unsigned char my_16bytes_key[16] = { 0 };
    for (int i = 0; i < 16; ++i) {
        my_16bytes_key[i] = 0x30;
    }

    //生成openssl库使用的秘钥结构体
    AES_KEY aes_key;
    if (AES_set_encrypt_key(my_16bytes_key, 128, &aes_key) < 0) {
        fprintf(stderr, "Unable to set encryption key in AES\n");
        exit(-1);
    }

    //初始向量，实际上就是一个16字节的字节序列，可以是任意值。
    unsigned char iv[AES_BLOCK_SIZE];        // init vector
    for (int i = 0; i < AES_BLOCK_SIZE; ++i) {
        iv[i] = 0;
    }

    //存放解密后的明文数据。因为明文和密文长度一致，当然知道解密后也要用len字节的内存。
    int len = 256;
    //int len = strlen((char *)cryper);
    unsigned char* decryptText = new unsigned char[len];

    //初始向量。应与加密时的初始向量一致。
    //这里再次给iv数组赋值的原因：前一次调用AES_cbc_encrypt后，iv的内容已被改变了。这里是恢复为期望的值。
    for (int i = 0; i < AES_BLOCK_SIZE; ++i)
    {
        iv[i] = 0;
    }

    //初始化秘钥结构体。应该与加密时的秘钥相同，这才叫“对称加密”
    if (AES_set_decrypt_key(my_16bytes_key, 128, &aes_key) < 0) {
        fprintf(stderr, "Unable to set decryption key in AES\n");
        exit(-1);
    }

    //解密。
    AES_cbc_encrypt(*cryper, decryptText, len, &aes_key, iv, AES_DECRYPT);

    // print
    //printf("plainText = %s\n", plainText);
    //printf("\n");
    printf("decryptText = %s\n", decryptText);
    plainStr = new char[len];
    strcpy(plainStr, (char *)decryptText);

    return;
}

