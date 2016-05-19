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

import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class HandleCreation extends AbstractAction {
    
    private static final Logger logger = LoggerFactory.getLogger(HandleCreation.class.getName());

    @Override
    public boolean perform(Context context) throws DepositException {
        
        String fedora = this.getParameter("fedoraServer");
        
        String fid = context.getSIP().getFID().toString().replaceAll("#.*","");
        String dsid = context.getSIP().getFID().getRawFragment().replaceAll("@.*","");
        String asof = context.getSIP().getFID().getRawFragment().replaceAll(".*@","");
        
        logger.info(" create handle["+context.getSIP().getPID()+"] -> URI["+fedora+"/objects/"+fid+"/datastreams/"+dsid+"/content?asOfDateTime="+asof+"]");
        
        for (Resource res:context.getSIP().getResources()) {
            String rfid = res.getFID().toString().replaceAll("#.*","");
            String rdsid = res.getFID().getRawFragment().replaceAll("@.*","");
            String rasof = res.getFID().getRawFragment().replaceAll(".*@","");

            logger.info(" create handle["+res.getPID()+"] -> URI["+fedora+"/objects/"+rfid+"/datastreams/"+rdsid+"/content?asOfDateTime="+rasof+"]");
        }
        
        return true;
    }
    
}
