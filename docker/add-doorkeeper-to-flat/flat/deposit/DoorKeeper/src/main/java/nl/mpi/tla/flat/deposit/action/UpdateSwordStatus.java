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
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Properties;
import java.util.logging.Level;
import java.util.logging.Logger;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;

/**
 *
 * @author menzowi
 */
public class UpdateSwordStatus extends AbstractAction {
    
    @Override
    public boolean perform(Context context) throws DepositException {
        String pfile = context.getProperty("props", "").toString();
        Path ppath = Paths.get(pfile);
        if (Files.exists(ppath) && Files.isReadable(ppath)) {
            Properties props = new Properties();
            try {
                props.load(new FileInputStream(ppath.toFile()));
                Boolean status = context.getFlow().getStatus();
                if (status == null) {
                    props.setProperty("state.label", "FAILED");
                    props.setProperty("state.description", "Deposit in the archive failed!");
                } else if (!status.booleanValue()) {
                    props.setProperty("state.label", "REJECTED");
                    props.setProperty("state.description", "The archive rejected the deposit!");
                } else {
                    props.setProperty("state.label", "ARCHIVED");
                    props.setProperty("state.description", "Deposit in the archive succeeded.");
                }
            } catch (IOException ex) {
                throw new DepositException(ex);
            }
        }
        return true;
    }
    
}
