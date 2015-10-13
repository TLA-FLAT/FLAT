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
public class WorkspaceLogCleanup extends AbstractAction {
    
    private static final Logger logger = LoggerFactory.getLogger(WorkspaceLogCleanup.class.getName());
    
    @Override
    public boolean perform(Context context) {
        return true;
    }
    
}
