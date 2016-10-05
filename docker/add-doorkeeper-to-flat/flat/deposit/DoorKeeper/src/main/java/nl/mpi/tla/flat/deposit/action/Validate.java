/*
 * Copyright (C) 2016 menzowi
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

import eu.clarin.cmdi.validator.CMDISchemaLoader;
import eu.clarin.cmdi.validator.CMDIValidationHandlerAdapter;
import eu.clarin.cmdi.validator.CMDIValidationReport;
import eu.clarin.cmdi.validator.CMDIValidator;
import eu.clarin.cmdi.validator.CMDIValidatorConfig;
import eu.clarin.cmdi.validator.CMDIValidatorException;
import eu.clarin.cmdi.validator.CMDIValidatorInitException;
import eu.clarin.cmdi.validator.SimpleCMDIValidatorProcessor;
import java.io.File;
import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.logging.Level;

import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import org.apache.commons.io.FileUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class Validate extends AbstractAction {
    
    private static final Logger logger = LoggerFactory.getLogger(Deposit.class.getName());
    
    @Override
    public boolean perform(Context context) throws DepositException {
        try {
            String schemaCache = getParameter("schemaCache","./cache");
            String rules = getParameter("rules");
            
            File cache = new File(schemaCache);
            if (!cache.exists())
                 FileUtils.forceMkdir(cache);

            Handler handler =  new Handler();
            CMDIValidatorConfig.Builder builder = new CMDIValidatorConfig.Builder(context.getSIP().getBase(), handler).socketTimeout(0);
            if (rules==null)
                builder = builder.disableSchematron();
            else
                builder = builder.schematronSchemaFile(Paths.get(rules).toFile());
            builder.schemaLoader(new CMDISchemaLoader(cache));
            CMDIValidatorConfig config = builder.build();
            
            CMDIValidator validator = new CMDIValidator(config, context.getSIP().getBase(), handler);
            SimpleCMDIValidatorProcessor processor = new SimpleCMDIValidatorProcessor();
            processor.process(validator);
            
            return (handler.result>0);
        } catch (CMDIValidatorInitException | CMDIValidatorException | IOException ex) {
            throw new DepositException(ex);
        }
    }
    
    private static class Handler extends CMDIValidationHandlerAdapter {
        
        protected int result;
        
        public Handler() {
            super();
            this.result = 0;
        }
        
        @Override
        public void onValidationReport(final CMDIValidationReport report)
                throws CMDIValidatorException {
            final File file = report.getFile();
            int skip = 0;
            switch (report.getHighestSeverity()) {
            case INFO:
                logger.info("{} is valid", file);
                result = 2;
                break;
            case WARNING:
                logger.warn("{} is valid (with warnings):", file);
                for (CMDIValidationReport.Message msg : report.getMessages()) {
                    if (msg.getMessage().contains("Failed to read schema document ''")) {
                        skip++;
                        continue;
                    }
                    if ((msg.getLineNumber() != -1) && (msg.getColumnNumber() != -1)) {
                        logger.warn(" ({}) {} [line={}, column={}]", msg.getSeverity().getShortcut(), msg.getMessage(), msg.getLineNumber(), msg.getColumnNumber());
                    } else {
                        logger.warn(" ({}) {}", msg.getSeverity().getShortcut(), msg.getMessage());
                    }
                }
                result = 1;
                break;
            case ERROR:
                logger.error("{} is invalid:", file);
                for (CMDIValidationReport.Message msg : report.getMessages()) {
                    if (msg.getMessage().contains("Failed to read schema document ''")) {
                        skip++;
                        continue;
                    }
                    if ((msg.getLineNumber() != -1) && (msg.getColumnNumber() != -1)) {
                        logger.error(" ({}) {} [line={}, column={}]", msg.getSeverity().getShortcut(), msg.getMessage(), msg.getLineNumber(), msg.getColumnNumber());
                    } else {
                        logger.error(" ({}) {}", msg.getSeverity().getShortcut(), msg.getMessage());
                    }
                }
                result = 0;
                break;
            default:
                throw new CMDIValidatorException("unexpected severity: " +
                        report.getHighestSeverity());
            } // switch
            if (skip>0)
                logger.warn("Skipped [{}] warnings due to lax validation of foreign namespaces", skip);
        }
    } // class Handler    

}


