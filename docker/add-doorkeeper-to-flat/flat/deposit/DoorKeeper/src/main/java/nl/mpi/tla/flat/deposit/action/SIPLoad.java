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

import java.io.File;
import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.SIP;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


/**
 *
 * @author menzowi
 */
public class SIPLoad extends AbstractAction {
    
    private static final Logger logger = LoggerFactory.getLogger(SIPLoad.class.getName());
    
    @Override
    public boolean perform(Context context) throws DepositException {
        XdmValue work = context.getProperty("work",".");
        if (work == null) {
            logger.error("no work directory specified!");
            return false;
        }

        File wd = new File(work.toString());
        if (!wd.isDirectory()) {
            logger.error("work["+wd+"] is not a directory!");
            return false;
        }
        if (!wd.canRead()) {
            logger.error("work["+wd+"] directory cannot be read!");
            return false;
        }
        if (!wd.canWrite()) {
            logger.error("work["+wd+"] directory cannot be written!");
            return false;
        }

        File mr = wd.toPath().resolve("./metadata/record.cmdi").toFile();
        if (!mr.isFile()) {
            logger.error("record["+mr+"] is not a file!");
            return false;
        }
        if (!mr.canRead()) {
            logger.error("record["+mr+"] file cannot be read!");
            return false;
        }
        if (!mr.canWrite()) {
            logger.error("work["+mr+"] file cannot be written!");
            return false;
        }

        context.setSIP(new SIP(mr));

        return true;
    }
    
}
