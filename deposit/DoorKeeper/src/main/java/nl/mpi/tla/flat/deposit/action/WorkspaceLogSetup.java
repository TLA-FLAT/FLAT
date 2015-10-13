/*
 * Copyright (C) 2015 menzowi
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
package nl.mpi.tla.flat.deposit.action;

import ch.qos.logback.classic.LoggerContext;
import ch.qos.logback.classic.joran.JoranConfigurator;
import ch.qos.logback.core.joran.spi.JoranException;
import ch.qos.logback.core.util.StatusPrinter;
import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import nl.mpi.tla.flat.deposit.Context;
import org.apache.commons.io.FileUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class WorkspaceLogSetup extends AbstractAction {
    
    private static final Logger logger = LoggerFactory.getLogger(WorkspaceLogSetup.class.getName());
    
    @Override
    public boolean perform(Context context) {
        try {
            File dir = new File(getParameter("dir","./logs"));
            if (!dir.exists())
                FileUtils.forceMkdir(dir);
            
            File logback = dir.toPath().resolve("./logback.xml").toFile();
            
            if (!logback.exists()) {
                // create {$work}/logs/logback.xml
                PrintWriter out = new PrintWriter(logback);
                out.print(
                    "<configuration>\n" +
                    "	<appender name=\"FILE\" class=\"ch.qos.logback.core.FileAppender\">\n" +
                    "		<file>" + dir + "/logfile.log</file>\n" +
                    "		<append>true</append>\n" +
                    "		<encoder>\n" +
                    "			<pattern>%d{HH:mm:ss.SSS} [%thread] %-5level %logger{36} : %msg%n</pattern>\n" +
                    "		</encoder>\n" +
                    "	</appender>\n" +
                    "	<root level=\"debug\">\n" +
                    "		<appender-ref ref=\"FILE\" />\n" +
                    "	</root>\n" +
                    "</configuration>"
                );
                out.close();
            }
            
            Logger logger = LoggerFactory.getLogger(nl.mpi.tla.flat.deposit.Flow.class);
            LoggerContext logctxt = (LoggerContext) LoggerFactory.getILoggerFactory();
            try {
                JoranConfigurator configurator = new JoranConfigurator();
                configurator.setContext(logctxt);
                configurator.doConfigure(logback.toString());
            } catch (JoranException je) {
                // StatusPrinter will handle this
            }
            StatusPrinter.printInCaseOfErrorsOrWarnings(logctxt);
            logger.info("\"Welcome to FLAT!\"\n" +
                "\"Relax,\" said the door keeper,\n" +
                "\"We are programmed to receive.\n" +
                "You can check-out any time you like,\n" +
                "But you can never leave!\""
            );
        } catch (IOException ex) {
            this.logger.error("Couldn't setup the deposit log!",ex);
            return false;
        }
        return true;
    }
    
}
