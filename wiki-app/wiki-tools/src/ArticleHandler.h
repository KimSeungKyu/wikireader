/*
 * Wiki Handling Tool
 *
 * Copyright (C) 2008 Openmoko Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ArticleHandler_h
#define ArticleHandler_h

#include <Article.h>

#include <QFile>

/**
 * Called whenever a WikiMedia has been parsed. This allows
 * to easily hook in new filters/converters.
 */
class ArticleHandler {
public:
    virtual ~ArticleHandler();

    virtual void parsingStarts() = 0;
    virtual void handleArticle(const Article&) = 0;
    virtual void parsingFinished() = 0;
};

class FileOutputArticleHandler : public ArticleHandler {
public:
    FileOutputArticleHandler(const QString& output);

    void parsingStarts();
    void parsingFinished();

protected:
    void openFile(const QString&);
    void closeCurrentFile();
    QString m_fileName;
    QFile m_file;
};

#endif