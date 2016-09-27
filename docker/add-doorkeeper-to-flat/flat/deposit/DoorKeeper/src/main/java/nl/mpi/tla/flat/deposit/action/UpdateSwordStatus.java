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

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Properties;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class UpdateSwordStatus extends AbstractAction {
    
    private static final org.slf4j.Logger logger = LoggerFactory.getLogger(UpdateSwordStatus.class.getName());
    
    @Override
    public boolean perform(Context context) throws DepositException {
        String pfile = this.getParameter("props");
        Path ppath = Paths.get(pfile);
        logger.debug("SWORD status properties["+ppath.toAbsolutePath()+"]");
        if (Files.exists(ppath) && Files.isReadable(ppath)) {
            Properties props = new Properties();
            try {
                FileInputStream in = new FileInputStream(ppath.toFile());
                props.load(in);
                in.close();
                Boolean status = context.getFlow().getStatus();
                if (status == null) {
                    props.setProperty("state.label", "FAILED");
                    props.setProperty("state.description", "Deposit in the archive failed!");
                    logger.debug("SWORD status updated to [FAILED]");
                } else if (!status.booleanValue()) {
                    props.setProperty("state.label", "REJECTED");
                    props.setProperty("state.description", "The archive rejected the deposit!");
                    logger.debug("SWORD status updated to [REJECTED]");
                } else {
                    props.setProperty("state.label", "ARCHIVED");
                    props.setProperty("state.description", "Deposit in the archive succeeded.");
                    logger.debug("SWORD status updated to [ARCHIVED]");
                }
                FileOutputStream out = new FileOutputStream(ppath.toFile());
                props.store(out,"SWORD SIP status");
                out.close();
            } catch (IOException ex) {
                throw new DepositException(ex);
            }
        }
        return true;
    }
    
}
