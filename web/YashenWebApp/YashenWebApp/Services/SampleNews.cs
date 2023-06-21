using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using YashenWebApp.Models;

namespace YashenWebApp.Services
{
    public class SampleNews
    {
        private const bool Deleted = false;
        private readonly IPublishNewsService publishNewsService_;
        public SampleNews(IPublishNewsService publishNewsService) =>
        publishNewsService_ = publishNewsService;

        private readonly string[] sampleTitles_ = new[]
        {
            "亚申PPSR-16E高通量平行反应釜系统申报省级首台套产品认定",
            "高温高压制备特种合成蜡和基础油RSC-600M-3C系统通过客户验收",
            "北京低碳清洁能源研究院门卓武博士一行来访亚申科技",
            "南非大学Bhekie B Mamba教授一行日前到亚申科技参观访问"
        };

        private readonly string[] sampleContents_ = new[]
        {
            "PPSR-16E高通量平行反应釜系统采用亚申科技国际独创的专有技术，在主要性能指标如温压控制精度、数据处理和分析软件平台等方面，处于国际领先水平。该系统填补了国内空白，将改变国外产品在高端设备市场的垄断地位，并在进口替代、卡脖子技术突破、可配置性强、客制化程度高、提供贴身的本地化服务，比国际竞争对手具有明显优势。@@ PPSR-16E高通量平行反应釜系统可广泛应用于生产企业、研发机构和高校科研。",
            "2022年1月21日亚申科技向柯桥绿色能源材料联合实验室交付RSC-600M-3C反应系统，并成功通过验收。@@ 该反应系统包含固定床反应器和连续搅拌反应器子系统，用于合成气转化等气固反应工业催化剂考评。RSC-600M-3C系统采用模块集成方式，具有很强的灵活性和可扩展性，为新增工艺设备提供了便利。",
            "2023年5月10日，北京低碳清洁能源研究院煤间接液化重大专项中心主任、国家重点研发计划项目负责人门卓武博士、南非科学院院士、Diane院士工作站主要成员刘歆颖教授到亚申科技进行了工作访问和学术交流。@@ 门卓武博士、刘歆颖教授深入了解了在亚申科技的Diane院士工作站的工作情况。参观了亚申科技重点实验室，与有关项目负责人就产业化技术的开发成果做了学术交流。",
            "2022年8月16日南非大学Bhekie B Mamba教授一行到亚申科技参观访问和学术与技术交流。\r\n                    Mamba教授为南非大学科学工程与技术学院执行院长，英国皇家化工学会研究员，国际知名环保领域技术专家。Mamba教授参观了亚申科技的实验室和技术放大基地，了解了亚申科技正在进行的研发和产业化推进项目，双方就可以合作的环保技术进行了深入交流。@@ 亚申科技总裁权华博士就成功开发的一系列首创的高通量系统平台，将这一全新的材料开发模式应用于洁净能源领域新材料（特别是新型工业催化剂和新工艺）的开发，以加速推动传统能源产业结构向节能环保产业发展转型中关键技术的突破方面做了深入浅出的介绍。\r\n                    四川轻化工大学教授、博导马建军、姜彩荣等专家，以及政府有关领导陪同参加了参观访问。来宾们对亚申科技的无烟柴油、紧凑型费托产业化技术和高通量新材料及工艺开发系统装备表达了浓厚的兴趣。"
        };

        // Npgsql <> EF6.0+ mapping and translateion https://www.npgsql.org/efcore/mapping/translations.html
        // .NET                        ||   sql
        // dateTime.ToUniversalTime    ||   dateTime::timestamptz 
        // ddateTime.ToLocalTime       ||   dateTime::timestamp     
        private readonly DateTime[] samplePublishDate_ = 
        {
            new DateTime(2022, 10, 12).ToUniversalTime(), 
            new DateTime(2022, 02, 18).ToUniversalTime(),
            new DateTime(2023, 05, 15).ToUniversalTime(),
            new DateTime(2022, 08, 20).ToUniversalTime(),
        };

        private readonly string[] samplePicturePath_ = new[]
        {
            "img/news_jiahua1.jpg",
            "img/news_zhijiangxueyuan.jpg",
            "",
            ""
        };

        private readonly string[] authors = new[]
        {
            "abc@yashentech.com",
            "def@yashentech.com",
            "abc@yashentech.com",
            "def@yashentech.com"
        };

        public void CreateSampleNews()
        {
            List<NewsItem> newsList = new();
            for (int i = 0; i < 4; i++)
            {
                newsList.Add(new NewsItem(id: Guid.NewGuid(),
                                          sampleTitles_[i],
                                          sampleContents_[i],
                                          samplePublishDate_[i],
                                          samplePicturePath_[i],
                                          authors[i],
                                          Deleted,
                                          DateTime.Now.ToUniversalTime()));
            }
            publishNewsService_.AddRangeAsync(newsList);
        }
    }
}
